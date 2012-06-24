#include "qxtgroupbox.h"
/****************************************************************************
** Copyright (c) 2006 - 2011, the LibQxt project.
** See the Qxt AUTHORS file for a list of authors and copyright holders.
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**     * Redistributions of source code must retain the above copyright
**       notice, this list of conditions and the following disclaimer.
**     * Redistributions in binary form must reproduce the above copyright
**       notice, this list of conditions and the following disclaimer in the
**       documentation and/or other materials provided with the distribution.
**     * Neither the name of the LibQxt project nor the
**       names of its contributors may be used to endorse or promote products
**       derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
** DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
** DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
** (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
** LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
** ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
** <http://libqxt.org>  <foundation@libqxt.org>
*****************************************************************************/


#include <QChildEvent>

class GroupBoxPrivate
{
public:
    friend class GroupBox;

    GroupBoxPrivate();
    bool collapsive;
    bool flat; // so we can restore it when expanding
};

GroupBoxPrivate::GroupBoxPrivate() : collapsive(true), flat(false)
{}


/*!
    \class QxtGroupBox
    \inmodule QxtWidgets
    \brief The QxtGroupBox widget is a collapsive and checkable QGroupBox.

    QxtGroupBox is a checkable group box automatically expanding/collapsing
    its content according to the check state. QxtGroupBox shows its children
    when checked and hides its children when unchecked.

    \image qxtgroupbox.png "Two QxtGroupBoxes - an expanded and a collapsed - on top of each other."
 */

/*!
    Constructs a new QxtGroupBox with \a parent.
 */
GroupBox::GroupBox(QWidget* parent)
        : QGroupBox(parent)
{
    qxt_d = new GroupBoxPrivate;
    setCheckable(true);
    setChecked(true);
    connect(this, SIGNAL(toggled(bool)), this, SLOT(setExpanded(bool)));

}

/*!
    Constructs a new QxtGroupBox with \a title and \a parent.
 */
GroupBox::GroupBox(const QString& title, QWidget* parent)
        : QGroupBox(title, parent)
{
    qxt_d = new GroupBoxPrivate;
    setCheckable(true);
    setChecked(true);
    connect(this, SIGNAL(toggled(bool)), this, SLOT(setExpanded(bool)));
}

/*!
    Destructs the group box.
 */
GroupBox::~GroupBox()
{
    delete qxt_d;
}

/*!
    \property QxtGroupBox::collapsive
    \brief whether the group box is collapsive

    The default value is \c true.
 */
bool GroupBox::isCollapsive() const
{
    return qxt_d->collapsive;
}

void GroupBox::setCollapsive(bool enable)
{
    if (qxt_d->collapsive != enable)
    {
        qxt_d->collapsive = enable;
        if (!enable)
            setExpanded(true);
        else if (!isChecked())
            setExpanded(false);
    }
}

/*!
    Sets the group box \a collapsed.

    A collapsed group box hides its children.

    \sa setExpanded(), QGroupBox::toggled()
 */
void GroupBox::setCollapsed(bool collapsed)
{
    setExpanded(!collapsed);
}

/*!
    Sets the group box \a expanded.

    An expanded group box shows its children.

    \sa setCollapsed(), QGroupBox::toggled()
 */
void GroupBox::setExpanded(bool expanded)
{
    if (qxt_d->collapsive || expanded)
    {
        // show/hide direct children
        foreach(QObject* child, children())
        {
            if (child->isWidgetType())
                static_cast<QWidget*>(child)->setVisible(expanded);
        }
        if (expanded) {
          setFlat(qxt_d->flat);
        } else {
          qxt_d->flat = isFlat();
          setFlat(true);
        }
    }
}

/*!
    \reimp
 */
void GroupBox::childEvent(QChildEvent* event)
{
    QObject* child = event->child();
    if (event->added() && child->isWidgetType())
    {
        QWidget* widget = static_cast<QWidget*>(child);
        if (qxt_d->collapsive && !isChecked())
            widget->hide();
    }
}
