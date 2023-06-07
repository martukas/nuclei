#include "ProxyStyle.h"
#include <QEvent>

ProxyStyle::ProxyStyle(QStyle *style) :
    QProxyStyle(style)
{
}

int ProxyStyle::styleHint(QStyle::StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const
{
    if (hint == QStyle::SH_ItemView_ActivateItemOnSingleClick)
        return 1;

    return QProxyStyle::styleHint(hint, option, widget, returnData);
}
