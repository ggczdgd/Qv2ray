/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "QvAutoCompleteTextEdit.h"

#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QApplication>
#include <QCompleter>
#include <QKeyEvent>
#include <QModelIndex>
#include <QScrollBar>
#include <QStringListModel>
#include <QtDebug>

namespace Qv2ray::ui::widgets
{
    AutoCompleteTextEdit::AutoCompleteTextEdit(const QString &prefix, const QStringList &sourceStrings, QWidget *parent) : QTextEdit(parent)
    {
        this->prefix = prefix;
        c = new QCompleter(this);
        c->setModel(new QStringListModel(sourceStrings, c));
        c->setWidget(this);
        c->setCompletionMode(QCompleter::PopupCompletion);
        c->setCaseSensitivity(Qt::CaseInsensitive);
        QObject::connect(c, QOverload<const QString &>::of(&QCompleter::activated), this, &AutoCompleteTextEdit::insertCompletion);
    }

    AutoCompleteTextEdit::~AutoCompleteTextEdit()
    {
    }

    void AutoCompleteTextEdit::insertCompletion(const QString &completion)
    {
        QTextCursor tc = textCursor();
        int extra = completion.length() - c->completionPrefix().length();
        tc.movePosition(QTextCursor::Left);
        tc.movePosition(QTextCursor::EndOfWord);
        tc.insertText(completion.right(extra));
        setTextCursor(tc);
    }

    QString AutoCompleteTextEdit::lineUnderCursor() const
    {
        QTextCursor tc = textCursor();
        tc.select(QTextCursor::LineUnderCursor);
        return tc.selectedText();
    }

    QString AutoCompleteTextEdit::wordUnderCursor() const
    {
        QTextCursor tc = textCursor();
        tc.select(QTextCursor::WordUnderCursor);
        return tc.selectedText();
    }

    void AutoCompleteTextEdit::focusInEvent(QFocusEvent *e)
    {
        if (c)
            c->setWidget(this);

        QTextEdit::focusInEvent(e);
    }

    void AutoCompleteTextEdit::keyPressEvent(QKeyEvent *e)
    {
        if (c && c->popup()->isVisible())
        {
            // The following keys are forwarded by the completer to the widget
            switch (e->key())
            {
                case Qt::Key_Enter:
                case Qt::Key_Return:
                case Qt::Key_Escape:
                case Qt::Key_Tab:
                case Qt::Key_Backtab: e->ignore(); return; // let the completer do default behavior

                default: break;
            }
        }

        const bool isShortcut = (e->modifiers().testFlag(Qt::ControlModifier) && e->key() == Qt::Key_Space); // CTRL+Space

        if (!c || !isShortcut) // do not process the shortcut when we have a
                               // completer
            QTextEdit::keyPressEvent(e);

        const bool ctrlOrShift = e->modifiers().testFlag(Qt::ControlModifier) || e->modifiers().testFlag(Qt::ShiftModifier);

        if (!c || (ctrlOrShift && e->text().isEmpty()))
            return;

        static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-="); // end of word
        const bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
        if (!isShortcut &&
            (hasModifier || e->text().isEmpty() || !lineUnderCursor().startsWith(prefix) /* || eow.contains(e->text().right(1))*/))
        {
            c->popup()->hide();
            return;
        }

        auto word = wordUnderCursor();
        if (word != c->completionPrefix())
        {
            c->setCompletionPrefix(word);
            c->popup()->setCurrentIndex(c->completionModel()->index(0, 0));
        }

        QRect cr = cursorRect();
        cr.setWidth(c->popup()->sizeHintForColumn(0) + c->popup()->verticalScrollBar()->sizeHint().width());
        c->complete(cr); // popup it up!
    }
} // namespace Qv2ray::ui::widgets