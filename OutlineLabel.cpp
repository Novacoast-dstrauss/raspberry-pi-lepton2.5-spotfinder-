#include "OutlineLabel.h"

OutlineLabel::OutlineLabel(QWidget *parent) : QLabel(parent)
{
}
OutlineLabel::~OutlineLabel()
{
}

//Overrides paintEvent to give the white text an outline
void OutlineLabel::paintEvent(QPaintEvent *event){
        QPainter painter(this);
        QFont font("Arial", 10, QFont::Bold, true);
        painter.setFont(font);
	
	//Simple 4 sided redrawing of text, as QLabel doesn't have a stroke effect.
        QRect top_rect = event->rect();
        QRect left = QRect(top_rect.left() + 2,top_rect.top(),top_rect.right() + 2,top_rect.bottom());
        QRect right = QRect(top_rect.left() - 2,top_rect.top(),top_rect.right() - 2,top_rect.bottom());
        QRect top = QRect(top_rect.left(),top_rect.top() - 2,top_rect.right(),top_rect.bottom() - 2);
        QRect bottom = QRect(top_rect.left(),top_rect.top() + 2,top_rect.right(),top_rect.bottom() + 2);

        painter.setPen(QPen(Qt::black));
        painter.drawText(top_rect, text());
        painter.drawText(top, text());
        painter.drawText(bottom, text());
        painter.drawText(left, text());
        painter.drawText(right, text());
        
	font.setBold(false);
	painter.setPen(QPen(Qt::white));
        painter.drawText(top_rect, text());

}

