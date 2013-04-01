#include "port_popup.h"

PopupMenu::PopupMenu(QWidget* parent)
  :QMenu(parent)
{
  acDisconnect = new QAction(tr("&Disconnect"), this);
  addAction(acDisconnect);

  addSeparator();
  acDefaultCable = new QAction(tr("De&fault Cable"), this);
  addAction(acDefaultCable);

  acGrayCable = new QAction(tr("Gr&ay Cable"), this);
  addAction(acGrayCable);

  acRedCable = new QAction(tr("&Red Cable"), this);
  addAction(acRedCable);

  acGreenCable = new QAction(tr("&Green Cable"), this);
  addAction(acGreenCable);

  acBlueCable = new QAction(tr("&Blue Cable"), this);
  addAction(acBlueCable);

  acYellowCable = new QAction(tr("&Yellow Cable"), this);
  addAction(acYellowCable);

  addSeparator();
  acSetJackColor = new QAction(tr("Set &Jack Color..."), this);
  addAction(acSetJackColor);

  acSetCableColor = new QAction(tr("Set &Cable Color..."), this);
  addAction(acSetCableColor);
}

PopupMenu::portAction PopupMenu::runAt(const QPoint& pos)
{
    portAction result;
    QAction* ac = exec(pos);

    if (ac == acDisconnect)
        result = paDisconnect;
    else if (ac ==acDefaultCable)
        result = paDefaultCable;
    else if (ac ==acGrayCable)
        result = paGrayCable;
    else if (ac ==acRedCable)
        result = paRedCable;
    else if (ac ==acGreenCable)
        result = paGreenCable;
    else if (ac ==acBlueCable)
        result = paBlueCable;
    else if (ac ==acYellowCable)
        result = paYellowCable;
    else if (ac ==acSetJackColor)
        result = paSetJackColor;
    else if (ac ==acSetCableColor)
        result = paSetCableColor;
    else
        result = paNone;

    return result;
}
