#include "BandwidthMonitorWidget.h"

#include <QComboBox>
#include <QFile>
#include <QHBoxLayout>
#include <QJsonObject>
#include <QLabel>
#include <QMap>
#include <QNetworkInterface>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include <optional>

namespace {

QString formatRate(double bytesPerSecond) {
    static const char* units[] = {"B/s", "KiB/s", "MiB/s", "GiB/s"};
    int unitIdx = 0;
    double value = bytesPerSecond;
    while (value >= 1024.0 && unitIdx < 3) {
        value /= 1024.0;
        ++unitIdx;
    }

    if (unitIdx == 0) {
        return QString::number(static_cast<quint64>(value)) + " " + units[unitIdx];
    }
    return QString::number(value, 'f', 1) + " " + units[unitIdx];
}

#if defined(Q_OS_LINUX)
struct NetCounters {
    quint64 rxBytes = 0;
    quint64 txBytes = 0;
};

QMap<QString, NetCounters> readLinuxCounters() {
    QMap<QString, NetCounters> counters;

    QFile proc("/proc/net/dev");
    if (!proc.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return counters;
    }

    while (!proc.atEnd()) {
        const QString line = QString::fromUtf8(proc.readLine()).trimmed();
        if (!line.contains(':')) {
            continue;
        }

        const QStringList sides = line.split(':');
        if (sides.size() != 2) {
            continue;
        }

        const QString iface = sides[0].trimmed();
        const QStringList fields = sides[1].simplified().split(' ');
        if (fields.size() < 16) {
            continue;
        }

        bool okRx = false;
        bool okTx = false;
        const quint64 rx = fields[0].toULongLong(&okRx);
        const quint64 tx = fields[8].toULongLong(&okTx);
        if (!okRx || !okTx) {
            continue;
        }

        counters.insert(iface, NetCounters{.rxBytes = rx, .txBytes = tx});
    }

    return counters;
}
#endif

class BandwidthMonitorView final : public QWidget {
    Q_OBJECT

public:
    explicit BandwidthMonitorView(QString selectedIface, QWidget* parent = nullptr)
        : QWidget(parent), selectedInterface_(std::move(selectedIface)) {
        auto* root = new QVBoxLayout(this);
        root->setContentsMargins(10, 10, 10, 10);
        root->setSpacing(8);

        auto* interfaceRow = new QHBoxLayout();
        auto* ifaceLabel = new QLabel("Interface:", this);
        interfaceBox_ = new QComboBox(this);
        interfaceRow->addWidget(ifaceLabel);
        interfaceRow->addWidget(interfaceBox_, 1);
        root->addLayout(interfaceRow);

        statusLabel_ = new QLabel(this);
        statusLabel_->setWordWrap(true);
        statusLabel_->setStyleSheet("color: #b0b0b0;");
        root->addWidget(statusLabel_);

        auto* downloadTitle = new QLabel("Download", this);
        auto* uploadTitle = new QLabel("Upload", this);
        downloadValue_ = new QLabel("--", this);
        uploadValue_ = new QLabel("--", this);

        downloadValue_->setStyleSheet("font-size: 22px; font-weight: 700;");
        uploadValue_->setStyleSheet("font-size: 22px; font-weight: 700;");

        root->addWidget(downloadTitle);
        root->addWidget(downloadValue_);
        root->addWidget(uploadTitle);
        root->addWidget(uploadValue_);
        root->addStretch();

        scanInterfaces();

#if defined(Q_OS_LINUX)
        connect(interfaceBox_, &QComboBox::currentTextChanged, this, [this](const QString& text) {
            selectedInterface_ = text;
            lastSample_.reset();
            emit selectedInterfaceChanged(selectedInterface_);
            updateReadings();
        });

        timer_ = new QTimer(this);
        timer_->setInterval(1000);
        connect(timer_, &QTimer::timeout, this, &BandwidthMonitorView::updateReadings);
        timer_->start();
        updateReadings();
#else
        Q_UNUSED(downloadTitle);
        Q_UNUSED(uploadTitle);
        statusLabel_->setText("Bandwidth monitoring is currently supported on Linux only.");
        interfaceBox_->setEnabled(false);
#endif
    }

    QString selectedInterface() const { return selectedInterface_; }

signals:
    void selectedInterfaceChanged(const QString& iface);

private:
    void scanInterfaces() {
        interfaceBox_->clear();

        const auto interfaces = QNetworkInterface::allInterfaces();
        for (const auto& iface : interfaces) {
            if (iface.flags().testFlag(QNetworkInterface::IsLoopBack)) {
                continue;
            }
            if (!iface.flags().testFlag(QNetworkInterface::IsUp)) {
                continue;
            }
            interfaceBox_->addItem(iface.humanReadableName(), iface.name());
        }

        if (interfaceBox_->count() == 0) {
            statusLabel_->setText("No active network interfaces found.");
            interfaceBox_->setEnabled(false);
            downloadValue_->setText("N/A");
            uploadValue_->setText("N/A");
            return;
        }

        interfaceBox_->setEnabled(true);

        int idx = -1;
        if (!selectedInterface_.isEmpty()) {
            idx = interfaceBox_->findData(selectedInterface_);
            if (idx < 0) {
                idx = interfaceBox_->findText(selectedInterface_);
            }
        }
        if (idx < 0) {
            idx = 0;
        }

        interfaceBox_->setCurrentIndex(idx);
        selectedInterface_ = interfaceBox_->currentData().toString();
        if (selectedInterface_.isEmpty()) {
            selectedInterface_ = interfaceBox_->currentText();
        }

        statusLabel_->setText(QString("Sampling %1 every second").arg(interfaceBox_->currentText()));
    }

#if defined(Q_OS_LINUX)
    void updateReadings() {
        if (interfaceBox_->count() == 0) {
            return;
        }

        const auto counters = readLinuxCounters();
        const QString ifaceName = interfaceBox_->currentData().toString();

        if (!counters.contains(ifaceName)) {
            statusLabel_->setText(QString("Could not read counters for %1").arg(interfaceBox_->currentText()));
            downloadValue_->setText("N/A");
            uploadValue_->setText("N/A");
            lastSample_.reset();
            return;
        }

        const auto current = counters.value(ifaceName);
        if (!lastSample_.has_value()) {
            lastSample_ = current;
            downloadValue_->setText("--");
            uploadValue_->setText("--");
            statusLabel_->setText(QString("Sampling %1 every second").arg(interfaceBox_->currentText()));
            return;
        }

        const quint64 rxDelta = (current.rxBytes >= lastSample_->rxBytes) ? (current.rxBytes - lastSample_->rxBytes) : 0;
        const quint64 txDelta = (current.txBytes >= lastSample_->txBytes) ? (current.txBytes - lastSample_->txBytes) : 0;

        downloadValue_->setText(formatRate(static_cast<double>(rxDelta)));
        uploadValue_->setText(formatRate(static_cast<double>(txDelta)));

        statusLabel_->setText(QString("Sampling %1 every second").arg(interfaceBox_->currentText()));
        lastSample_ = current;
    }
#endif

    QComboBox* interfaceBox_ = nullptr;
    QLabel* statusLabel_ = nullptr;
    QLabel* downloadValue_ = nullptr;
    QLabel* uploadValue_ = nullptr;
    QString selectedInterface_;
    QTimer* timer_ = nullptr;

#if defined(Q_OS_LINUX)
    std::optional<NetCounters> lastSample_;
#endif
};

} // namespace

BandwidthMonitorWidget::BandwidthMonitorWidget(QObject* parent)
    : QObject(parent) {}

void BandwidthMonitorWidget::initialize(dashboard::WidgetContext* context) {
    context_ = context;
    if (context_ && selectedInterface_.isEmpty()) {
        selectedInterface_ = context_->setting("selectedInterface").toString();
    }
}

QWidget* BandwidthMonitorWidget::createWidget(QWidget* parent) {
    auto* view = new BandwidthMonitorView(selectedInterface_, parent);

    connect(view, &BandwidthMonitorView::selectedInterfaceChanged, view, [this](const QString& iface) {
        selectedInterface_ = iface;
        if (context_) {
            context_->setSetting("selectedInterface", selectedInterface_);
        }
    });

    selectedInterface_ = view->selectedInterface();

    return view;
}

QJsonObject BandwidthMonitorWidget::serialize() const {
    QJsonObject json;
    if (!selectedInterface_.isEmpty()) {
        json.insert("selectedInterface", selectedInterface_);
    }
    return json;
}

void BandwidthMonitorWidget::deserialize(const QJsonObject& data) {
    selectedInterface_ = data.value("selectedInterface").toString(selectedInterface_);
}

dashboard::WidgetMetadata BandwidthMonitorWidget::metadata() const {
    return {
        .name = "Bandwidth Monitor",
        .version = "1.0.0",
        .author = "duh-dashboard",
        .description = "Real-time download/upload throughput for a selected network interface.",
        .minSize = QSize(220, 160),
        .maxSize = QSize(520, 420),
        .defaultSize = QSize(300, 220),
    };
}

#include "BandwidthMonitorWidget.moc"
