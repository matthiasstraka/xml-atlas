/* 
 * This file is part of XMLAtlas (https://github.com/glaure/xml-atlas)
 * Copyright (c) 2022 Gunther Laure <gunther.laure@gmail.com>.
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "xa_editor.h"
#include <QPainter>
#include <QTextBlock>


XAEditor::XAEditor(QWidget *parent) : QPlainTextEdit(parent)
{
    lineNumberArea = new LineNumberArea(this);

    connect(this, &XAEditor::blockCountChanged, this, &XAEditor::updateLineNumberAreaWidth);
    connect(this, &XAEditor::updateRequest, this, &XAEditor::updateLineNumberArea);
    connect(this, &XAEditor::cursorPositionChanged, this, &XAEditor::highlightCurrentLine);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}

int XAEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

    return space;
}

void XAEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void XAEditor::markSelectedRange(uint64_t offset, std::size_t length)
{
    QList<QTextEdit::ExtraSelection> extraSelections = {};

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection = {};

        QColor lineColor = QColor(Qt::yellow).lighter(160);

        selection.format.setBackground(lineColor);
      
#if 0
        // highlight selection
        selection.cursor = textCursor();
        selection.cursor.setPosition(offset, QTextCursor::MoveAnchor);
        selection.cursor.setPosition(static_cast<int>(offset + length), QTextCursor::KeepAnchor);
        selection.cursor.setCharFormat(selection.format);
        selection.cursor.clearSelection();
        extraSelections.append(selection);
#else
        // highlight complete line
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.setPosition(offset, QTextCursor::MoveAnchor);
        selection.cursor.clearSelection();
        extraSelections.append(selection);
#endif

    }

    setExtraSelections(extraSelections);
    setTextCursor(extraSelections.first().cursor);
}

void XAEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void XAEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void XAEditor::highlightCurrentLine()
{
    //QList<QTextEdit::ExtraSelection> extraSelections;

    //if (!isReadOnly()) {
    //    QTextEdit::ExtraSelection selection;

    //    QColor lineColor = QColor(Qt::yellow).lighter(160);

    //    selection.format.setBackground(lineColor);
    //    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    //    selection.cursor = textCursor();
    //    selection.cursor.clearSelection();
    //    extraSelections.append(selection);
    //}

    //setExtraSelections(extraSelections);
}

void XAEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);



    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());



    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

