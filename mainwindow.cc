/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2017 by N.T.WORKS                                       *
 *                                                                         *
 *   Licensed under GPLv2 or any later version                             *
 *                                                                         *
 ***************************************************************************/
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <memory>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  core_(std::make_unique<CPU::Core>()),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);
}

MainWindow::~MainWindow()
{
  delete ui;
}

auto MainWindow::OnQuit() -> void
{
  qInfo() << "Quit ...";
  close();
}

auto MainWindow::OnReset() -> void
{
  qInfo() << "Reset ...";
  core_->Reset();
}

auto MainWindow::OnStart() -> void
{
  qInfo() << "Start ...";
  core_->Run();
}

auto MainWindow::OnStop() -> void
{
  qInfo() << "Stop ...";
  core_->Stop();
}

/*
 * slots
 */
void MainWindow::on_actionQuit_triggered()
{
  qInfo() << "On Quit";
  OnQuit();
}

void MainWindow::on_actionStart_triggered()
{
  qInfo() << "On Start";
  OnStart();
}

void MainWindow::on_actionStop_triggered()
{
  qInfo() << "On Stop";
  OnStop();
}

void MainWindow::on_actionReset_triggered()
{
  qInfo() << "On Reset";
  OnReset();
}
