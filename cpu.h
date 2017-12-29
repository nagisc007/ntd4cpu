/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2017 by N.T.WORKS                                       *
 *                                                                         *
 *   Licensed under GPLv2 or any later version                             *
 *                                                                         *
 ***************************************************************************/
#ifndef CPU_H
#define CPU_H

#include <memory>
#include <QByteArray>
#include <QMap>
#include <QObject>
#include <QDebug>
#include <QThread>

namespace CPU {

enum class RegType {
  PC,
  IN,
  OUT,
  A,
  B,
  Memory,
  Constant,
};

enum class OpeCode {
  ADD,
  MOV,
  JMP,
  JNC,
  IN,
  OUT,
};

// opecode/ operand
// code, Rd, R1, R2
using ope_code = std::tuple<OpeCode, RegType, RegType, RegType>;

class Ram
{
private:
  std::unique_ptr<QByteArray> data_;
public:
  explicit Ram(const int& = 0x10);
  auto operator()(const int& addr, const char& val);
  auto operator()(const int& addr, const int& val);
  constexpr auto Read(const int& addr) const;
  void Reset();
};

class ProgramCounter
{
private:
  int counter_;
public:
  explicit ProgramCounter(const int& = 0);
  constexpr auto operator()(const int& v);
  constexpr auto operator()();
  void Increment();
};

class StackPointer
{
private:
  int pointer_;
public:
  explicit StackPointer(const int& = 0);
  constexpr auto operator()(const int& p);
  constexpr auto Read() const;
};

class ALU
{
public:
  constexpr auto operator()(const int&, const int&) const;
};

class Selector
{
public:
  constexpr auto operator()(const RegType& type, const Ram& regs) const;
};

class Core;

class EmuThread: public QThread
{
  Q_OBJECT
public:
  explicit EmuThread(Core*);
  void run() override;
  void RequestStop();
  void SetRunning();
  void SetCycle(const int&);
public slots:
  void OnExecStep();
private:
  Core* core_;
  bool is_running_ = false;
  int cycle_;
};

class Core : public QObject
{
  Q_OBJECT
public:
  explicit Core(QObject *parent = nullptr);
  void Reset();
  void Run();
  bool Step();
  void Stop();
  void InstallRamenTimer();
  void SetCycle(const int&);

public slots:
private:
  bool is_running_ = false;
  std::unique_ptr<Ram> prog_rom_;
  std::unique_ptr<Ram> register_;
  std::unique_ptr<ProgramCounter> prog_counter_;
  std::unique_ptr<StackPointer> stack_pointer_;
  std::unique_ptr<Ram> prog_status_;
  std::unique_ptr<QMap<char, ope_code>> decoder_;
  int cycle_;
  std::unique_ptr<EmuThread> emu_thread_;
};

}  // namespace CPU

#endif // CPU_H
