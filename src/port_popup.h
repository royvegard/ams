#ifndef PORT_POPUP_H
#define PORT_POPUP_H

#include <QAction>
#include <QMenu>

class PopupMenu: public QMenu {
  Q_OBJECT

public:
    enum portAction {
        paNone = 0,
        paDisconnect,
        paDefaultCable,
        paGrayCable,
        paRedCable,
        paGreenCable,
        paBlueCable,
        paYellowCable,
        paSetJackColor,
        paSetCableColor
    };

  PopupMenu(QWidget* parent = NULL);
  PopupMenu::portAction runAt(const QPoint&);

private:
  QAction* acDisconnect;
  QAction* acDefaultCable;
  QAction* acGrayCable;
  QAction* acRedCable;
  QAction* acGreenCable;
  QAction* acBlueCable;
  QAction* acYellowCable;
  QAction* acSetJackColor;
  QAction* acSetCableColor;
};

#endif
