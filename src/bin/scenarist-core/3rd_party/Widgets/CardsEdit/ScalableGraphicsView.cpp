#include "ScalableGraphicsView.h"

#include <QApplication>
#include <QEvent>
#include <QGestureEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QScrollBar>
#include <QWheelEvent>


ScalableGraphicsView::ScalableGraphicsView(QWidget *parent) :
    QGraphicsView(parent)
{
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    setTransformationAnchor(AnchorUnderMouse);

    //
    // Отслеживаем жесты
    //
    grabGesture(Qt::PinchGesture);
    grabGesture(Qt::SwipeGesture);
}

void ScalableGraphicsView::zoomIn()
{
    scaleView(qreal(0.1));
}

void ScalableGraphicsView::zoomOut()
{
    scaleView(qreal(-0.1));
}

bool ScalableGraphicsView::event(QEvent* _event)
{
    //
    // Определяем особый обработчик для жестов
    //
    if (_event->type() == QEvent::Gesture) {
        gestureEvent(static_cast<QGestureEvent*>(_event));
        return true;
    }

    //
    // Прочие стандартные обработчики событий
    //
    return QGraphicsView::event(_event);
}

void ScalableGraphicsView::gestureEvent(QGestureEvent* _event)
{
    //
    // Жест масштабирования
    //
    if (QGesture* gesture = _event->gesture(Qt::PinchGesture)) {
        if (QPinchGesture* pinch = qobject_cast<QPinchGesture *>(gesture)) {
            //
            // При масштабировании за счёт жестов приходится немного притормаживать
            // т.к. события приходят слишком часто и при обработке каждого события
            // пользователю просто невозможно корректно настроить масштаб
            //

            const int INERTION_BREAK_STOP = 4;
            qreal zoomDelta = 0;
            if (pinch->scaleFactor() > 1) {
                if (m_gestureZoomInertionBreak < 0) {
                    m_gestureZoomInertionBreak = 0;
                } else if (m_gestureZoomInertionBreak >= INERTION_BREAK_STOP) {
                    m_gestureZoomInertionBreak = 0;
                    zoomDelta = 0.1;
                } else {
                    ++m_gestureZoomInertionBreak;
                }
            } else if (pinch->scaleFactor() < 1) {
                if (m_gestureZoomInertionBreak > 0) {
                    m_gestureZoomInertionBreak = 0;
                } else if (m_gestureZoomInertionBreak <= -INERTION_BREAK_STOP) {
                    m_gestureZoomInertionBreak = 0;
                    zoomDelta = -0.1;
                } else {
                    --m_gestureZoomInertionBreak;
                }
            } else {
                //
                // При обычной прокрутке часто приходит событие с scaledFactor == 1,
                // так что просто игнорируем их
                //
            }

            //
            // Если необходимо масштабируем и перерисовываем представление
            //
            const bool needZoomIn = pinch->scaleFactor() > 1;
            const bool needZoomOut = pinch->scaleFactor() < 1;
            if (zoomDelta != 0 && needZoomIn) {
                zoomIn();
            } else if (zoomDelta != 0 && needZoomOut) {
                zoomOut();
            }

            _event->accept();
        }
    }
}

void ScalableGraphicsView::wheelEvent(QWheelEvent* _event)
{
#ifdef Q_OS_MAC
    const qreal ANGLE_DIVIDER = 2.;
#else
    const qreal ANGLE_DIVIDER = 120.;
#endif
    //
    // Собственно масштабирование
    //
    if (_event->modifiers() & Qt::ControlModifier) {
        if (_event->orientation() == Qt::Vertical) {
            //
            // zoomRange > 0 - масштаб увеличивается
            // zoomRange < 0 - масштаб уменьшается
            //
            const qreal zoom = _event->angleDelta().y() / ANGLE_DIVIDER;
            if (zoom > 0) {
                zoomIn();
            } else if (zoom < 0) {
                zoomOut();
            }

            _event->accept();
        }
    }
    //
    // В противном случае прокручиваем редактор
    //
    else {
        QGraphicsView::wheelEvent(_event);
    }
}

void ScalableGraphicsView::keyPressEvent(QKeyEvent* _event)
{
    if ((_event->key() == Qt::Key_Plus || _event->key() == Qt::Key_Equal)
        && _event->modifiers() & Qt::ControlModifier)
    {
        zoomIn();
        return;
    }

    if (_event->key() == Qt::Key_Minus
        && _event->modifiers() & Qt::ControlModifier)
    {
        zoomOut();
        return;
    }

    if (_event->key() == Qt::Key_Delete
        || _event->key() == Qt::Key_Backspace) {
        emit deletePressed();
        return;
    }

    if (_event->key() == Qt::Key_Space) {
        m_inScrolling = true;
        return;
    }

    QGraphicsView::keyPressEvent(_event);
}

void ScalableGraphicsView::keyReleaseEvent(QKeyEvent* _event)
{
    if (_event->key() == Qt::Key_Space) {
        m_inScrolling = false;
    }

    QGraphicsView::keyReleaseEvent(_event);
}

void ScalableGraphicsView::mousePressEvent(QMouseEvent* _event)
{
    if (m_inScrolling
        && _event->buttons() & Qt::LeftButton) {
        m_scrollingLastPos = _event->globalPos();
        QApplication::setOverrideCursor(QCursor(Qt::ClosedHandCursor));
        return;
    }

    QGraphicsView::mousePressEvent(_event);
}

void ScalableGraphicsView::mouseMoveEvent(QMouseEvent* _event)
{
    //
    // Если в данный момент происходит прокрутка полотна
    //
    if (m_inScrolling) {
        if (_event->buttons() & Qt::LeftButton) {
            const QPoint prevPos = m_scrollingLastPos;
            m_scrollingLastPos = _event->globalPos();
            horizontalScrollBar()->setValue(horizontalScrollBar()->value() + (prevPos.x() - m_scrollingLastPos.x()));
            verticalScrollBar()->setValue(verticalScrollBar()->value() + (prevPos.y() - m_scrollingLastPos.y()));
        }
        return;
    }

    QGraphicsView::mouseMoveEvent(_event);
}

void ScalableGraphicsView::mouseReleaseEvent(QMouseEvent* _event)
{
    if (m_inScrolling) {
        QApplication::restoreOverrideCursor();
    }

    QGraphicsView::mouseReleaseEvent(_event);
}

void ScalableGraphicsView::scaleView(qreal _factor)
{
    scale(1 + _factor, 1 + _factor);
    emit scaleChanged();
}
