/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2017 by N.T.WORKS                                       *
 *                                                                         *
 *   Licensed under GPLv2 or any later version                             *
 *                                                                         *
 ***************************************************************************/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "cpu.h"
#include <memory>
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();
  void OnQuit();
  void OnReset();
  void OnStart();
  void OnStop();
  std::unique_ptr<CPU::Core> core_;

private slots:
  void on_actionQuit_triggered();
  void on_actionStart_triggered();
  void on_actionStop_triggered();
  void on_actionReset_triggered();

private:
  Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
