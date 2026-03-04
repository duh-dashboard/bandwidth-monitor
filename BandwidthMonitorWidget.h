#pragma once

#include <dashboard/IWidget.h>

#include <QObject>
#include <QString>

class BandwidthMonitorWidget final : public QObject, public dashboard::IWidget {
    Q_OBJECT
    Q_INTERFACES(dashboard::IWidget)
    Q_PLUGIN_METADATA(IID IWidget_iid FILE "bandwidth-monitor.json")

public:
    explicit BandwidthMonitorWidget(QObject* parent = nullptr);

    void initialize(dashboard::WidgetContext* context) override;
    QWidget* createWidget(QWidget* parent) override;
    QJsonObject serialize() const override;
    void deserialize(const QJsonObject& data) override;
    dashboard::WidgetMetadata metadata() const override;

private:
    dashboard::WidgetContext* context_ = nullptr;
    QString selectedInterface_;
};
