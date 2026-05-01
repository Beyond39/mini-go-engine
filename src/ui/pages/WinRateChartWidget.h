#pragma once

#include <QWidget>
#include <QVector>

// [改进] 独立胜率曲线控件：GamePage 不再用占位 QFrame，而是用 QPainter 真实绘制曲线。
class WinRateChartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WinRateChartWidget(QWidget* parent = nullptr);

    void clear();
    void setRates(const QVector<double>& blackRates);
    void appendRate(double blackRate);

    const QVector<double>& rates() const;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QVector<double> m_blackRates;   // 黑棋胜率，范围 0~100

    double clampRate(double value) const;
    QPointF pointForIndex(int index, const QRectF& plotRect) const;
};
