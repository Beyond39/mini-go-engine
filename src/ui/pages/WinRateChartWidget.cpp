#include "WinRateChartWidget.h"

#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QFontMetrics>
#include <QtMath>

WinRateChartWidget::WinRateChartWidget(QWidget* parent)
    : QWidget(parent)
{
    setMinimumHeight(180);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void WinRateChartWidget::clear()
{
    m_blackRates.clear();
    update();
}

void WinRateChartWidget::setRates(const QVector<double>& blackRates)
{
    m_blackRates.clear();
    m_blackRates.reserve(blackRates.size());

    for (double value : blackRates) {
        m_blackRates.push_back(clampRate(value));
    }

    update();
}

void WinRateChartWidget::appendRate(double blackRate)
{
    m_blackRates.push_back(clampRate(blackRate));
    update();
}

const QVector<double>& WinRateChartWidget::rates() const
{
    return m_blackRates;
}

double WinRateChartWidget::clampRate(double value) const
{
    if (value < 0.0) {
        return 0.0;
    }
    if (value > 100.0) {
        return 100.0;
    }
    return value;
}

QPointF WinRateChartWidget::pointForIndex(int index, const QRectF& plotRect) const
{
    if (m_blackRates.isEmpty()) {
        return QPointF(plotRect.left(), plotRect.center().y());
    }

    const double rate = clampRate(m_blackRates[index]);

    double x = plotRect.center().x();
    if (m_blackRates.size() > 1) {
        x = plotRect.left() + plotRect.width() * index / double(m_blackRates.size() - 1);
    }

    const double y = plotRect.bottom() - plotRect.height() * rate / 100.0;
    return QPointF(x, y);
}

void WinRateChartWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QRectF outer = rect().adjusted(2, 2, -2, -2);
    const QRectF plotRect = outer.adjusted(44, 24, -16, -30);

    // [改进] 使用实色背景，避免默认控件透明叠加导致“脏色”。
    painter.setPen(QPen(QColor(211, 200, 186), 1.0));
    painter.setBrush(QColor(255, 253, 248));
    painter.drawRoundedRect(outer, 10, 10);

    painter.setPen(QPen(QColor(176, 164, 148), 1.0));
    painter.drawRect(plotRect);

    // 网格线与刻度
    QFont smallFont = painter.font();
    smallFont.setPointSize(8);
    painter.setFont(smallFont);

    for (int rate : {0, 25, 50, 75, 100}) {
        const double y = plotRect.bottom() - plotRect.height() * rate / 100.0;

        QColor gridColor = (rate == 50) ? QColor(134, 104, 72) : QColor(226, 218, 207);
        painter.setPen(QPen(gridColor, rate == 50 ? 1.3 : 1.0));
        painter.drawLine(QPointF(plotRect.left(), y), QPointF(plotRect.right(), y));

        painter.setPen(QColor(105, 94, 82));
        painter.drawText(QRectF(4, y - 8, 36, 16), Qt::AlignRight | Qt::AlignVCenter,
                         QString::number(rate) + "%");
    }

    painter.setPen(QColor(70, 58, 48));
    painter.drawText(QRectF(plotRect.left(), 4, plotRect.width(), 18),
                     Qt::AlignCenter, "黑棋胜率曲线");

    if (m_blackRates.isEmpty()) {
        painter.setPen(QColor(130, 119, 105));
        painter.drawText(plotRect, Qt::AlignCenter, "暂无胜率数据");
        return;
    }

    // 曲线
    QPainterPath path;
    path.moveTo(pointForIndex(0, plotRect));
    for (int i = 1; i < m_blackRates.size(); ++i) {
        path.lineTo(pointForIndex(i, plotRect));
    }

    // [改进] 曲线用清晰的深蓝色实线，不用半透明颜色。
    painter.setPen(QPen(QColor(45, 91, 150), 2.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(path);

    // 关键点
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(45, 91, 150));
    const int step = qMax(1, m_blackRates.size() / 12);
    for (int i = 0; i < m_blackRates.size(); i += step) {
        painter.drawEllipse(pointForIndex(i, plotRect), 3.2, 3.2);
    }

    // 最新点与标签
    const int lastIndex = m_blackRates.size() - 1;
    const QPointF lastPoint = pointForIndex(lastIndex, plotRect);
    const double lastRate = m_blackRates[lastIndex];

    painter.setBrush(QColor(209, 89, 65));
    painter.drawEllipse(lastPoint, 5.0, 5.0);

    painter.setPen(QColor(80, 58, 44));
    const QString label = QString("第%1手  黑%2%").arg(qMax(0, lastIndex)).arg(lastRate, 0, 'f', 1);
    QRectF labelRect(lastPoint.x() - 58, plotRect.bottom() + 6, 116, 20);
    if (labelRect.left() < plotRect.left()) {
        labelRect.moveLeft(plotRect.left());
    }
    if (labelRect.right() > plotRect.right()) {
        labelRect.moveRight(plotRect.right());
    }
    painter.drawText(labelRect, Qt::AlignCenter, label);
}
