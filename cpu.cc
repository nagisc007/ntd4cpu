/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2017 by N.T.WORKS                                       *
 *                                                                         *
 *   Licensed under GPLv2 or any later version                             *
 *                                                                         *
 ***************************************************************************/
#include "cpu.h"
#include <QDebug>
#include <QTimer>

namespace CPU {

Ram::Ram(const int& size): data_(std::make_unique<QByteArray>(size, qint8(0))) {}

auto Ram::operator()(const int& addr, const char& val)
{
  data_->operator[](addr) = val;
  return data_->at(addr);
}

auto Ram::operator()(const int& addr, const int& val)
{
  data_->operator[](addr) = static_cast<char>(val);
  return data_->at(addr);
}

constexpr auto Ram::Read(const int& addr) const
{
  return data_->at(addr);
}

auto Ram::Reset() -> void
{
  data_->fill(0);
}

ProgramCounter::ProgramCounter(const int& start): counter_(start) {}

constexpr auto ProgramCounter::operator()(const int& v)
{
  counter_ = v;
  return counter_;
}

constexpr auto ProgramCounter::operator()()
{
  return counter_;
}

auto ProgramCounter::Increment() -> void
{
  ++counter_;
}

StackPointer::StackPointer(const int& start): pointer_(start) {}

constexpr auto StackPointer::operator()(const int& p)
{
  pointer_ = p;
  return pointer_;
}

constexpr auto StackPointer::Read() const
{
  return pointer_;
}

constexpr auto ALU::operator()(const int& a, const int& b) const
{
  return a + b;
}

constexpr auto Selector::operator()(const RegType& type, const Ram& regs) const
{
  return type == RegType::A ? regs.Read(1):
         type == RegType::B ? regs.Read(2):
         type == RegType::Memory ? regs.Read(0) & 0x0f:
         type == RegType::IN ? regs.Read(3):
         0x00;
}

EmuThread::EmuThread(Core* core): core_(core) {}

auto EmuThread::SetRunning() -> void
{
  is_running_ = true;
}

void EmuThread::run()
{
  qInfo() << "run";
  auto timer = std::make_unique<QTimer>();
  connect(timer.get(), SIGNAL(timeout()), this, SLOT(OnExecStep()), Qt::DirectConnection);
  timer->setInterval(cycle_);
  timer->start();
  exec();
  timer->stop();
  return;
}

auto EmuThread::OnExecStep() -> void
{
  if (is_running_) {
    if (!core_->Step()) {
      is_running_ = false;
    }
  }
  return;
}

auto EmuThread::SetCycle(const int& cycle) -> void
{
  cycle_ = cycle;
}

auto EmuThread::RequestStop() -> void
{
  qInfo() << "Request Stop ...";
  is_running_ = false;
}

Core::Core(QObject *parent) : QObject(parent), emu_thread_(nullptr)
{
  prog_rom_ = std::make_unique<Ram>(16);  // 16byte
  register_ = std::make_unique<Ram>(5);  // I, A,B, In, Out
  prog_counter_ = std::make_unique<ProgramCounter>(0);  // pc
  stack_pointer_ = std::make_unique<StackPointer>(0);  // sp
  prog_status_ = std::make_unique<Ram>(1);  // Carry only
  cycle_ = 5;
  // init decoder
  decoder_ = std::make_unique<QMap<char, ope_code>>();
  decoder_->operator[](0x00) = std::make_tuple(OpeCode::ADD, RegType::A, RegType::A, RegType::Memory);
  decoder_->operator[](0x01) = std::make_tuple(OpeCode::MOV, RegType::A, RegType::B, RegType::Constant);
  decoder_->operator[](0x02) = std::make_tuple(OpeCode::IN, RegType::A, RegType::IN, RegType::Constant);
  decoder_->operator[](0x03) = std::make_tuple(OpeCode::MOV, RegType::A, RegType::A, RegType::Constant);
  decoder_->operator[](0x04) = std::make_tuple(OpeCode::MOV, RegType::B, RegType::A, RegType::Constant);
  decoder_->operator[](0x05) = std::make_tuple(OpeCode::ADD, RegType::B, RegType::B, RegType::Memory);
  decoder_->operator[](0x06) = std::make_tuple(OpeCode::IN, RegType::B, RegType::IN, RegType::Constant);
  decoder_->operator[](0x07) = std::make_tuple(OpeCode::MOV, RegType::B, RegType::B, RegType::Constant);
  decoder_->operator[](0x09) = std::make_tuple(OpeCode::OUT, RegType::OUT, RegType::B, RegType::Constant);
  decoder_->operator[](0x0b) = std::make_tuple(OpeCode::OUT, RegType::OUT, RegType::Memory, RegType::Constant);
  decoder_->operator[](0x0e) = std::make_tuple(OpeCode::JNC, RegType::PC, RegType::Memory, RegType::Constant);
  decoder_->operator[](0x0f) = std::make_tuple(OpeCode::JMP, RegType::PC, RegType::Memory, RegType::Constant);
  // test rom
  InstallRamenTimer();
  // connect thread
}

auto Core::SetCycle(const int& cycle) -> void
{
  cycle_ = cycle;
}

auto Core::InstallRamenTimer() -> void
{
  // ramen timer
  prog_rom_->operator()(0, 0b10110111);  // OUT 0111   # LED
  prog_rom_->operator()(1, 0b00000011);  // ADD A,0001
  prog_rom_->operator()(2, 0b11100001);  // JNC 0001 # loop 16 times
  prog_rom_->operator()(3, 0b00000011);  // ADD A,0001
  prog_rom_->operator()(4, 0b11100011);  // JNC 0011   # loop 16 times
  prog_rom_->operator()(5, 0b10110110);  // OUT 0110   # LED
  prog_rom_->operator()(6, 0b00000011);  // ADD A,0001
  prog_rom_->operator()(7, 0b11100110);  // JNC 0110   # loop 16 times
  prog_rom_->operator()(8, 0b00000001);  // ADD A,0001
  prog_rom_->operator()(9, 0b11101000);  // JNC 1000   # loop 16 times
  prog_rom_->operator()(10, 0b10110000);  // OUT 0000   # LED
  prog_rom_->operator()(11, 0b10110100);  // OUT 0100   # LED
  prog_rom_->operator()(12, 0b00000001);  // AND 0001
  prog_rom_->operator()(13, 0b11101010);  // JNC 1010   # loop 16 times
  prog_rom_->operator()(14, 0b10111000);  // OUT 1000   # LED
  prog_rom_->operator()(15, 0b11111111);  // JMP
  return;
}

auto Core::Reset() -> void
{
  is_running_ = false;
  register_->Reset();
  prog_counter_->operator()(0);
  stack_pointer_->operator()(0);
  return;
}

auto Core::Run() -> void
{
  // exists thread, so shutdown
  // if (emu_thread_ != nullptr) ...
  if (emu_thread_ != nullptr) return;

  const auto hz = 1000 / cycle_;
  emu_thread_ = std::make_unique<EmuThread>(this);
  emu_thread_->SetRunning();
  emu_thread_->SetCycle(hz);
  emu_thread_->run();
  // step loop
  //is_running_ = true;
  //while (is_running_) {
  //  if(!Step()) {
  //    break;
  //  }
  //  QThread::msleep(hz);
  //}
  qInfo() << "Exit of step";
  return;
}

auto Core::Step() -> bool
{
  // fetch
  register_->operator()(0, prog_rom_->Read(prog_counter_->operator()()));
  qInfo() << "STEP -- " << prog_counter_->operator()();
  // decode
  auto code = decoder_->operator[](register_->Read(0) >> 4 & 0x0f);
  Selector sel;
  auto r0 = sel(std::get<2>(code), *register_.get());
  auto r1 = sel(std::get<3>(code), *register_.get());
  int res = ALU()(r0,r1);
  if (res > 0x0f) {
    res &= 0x0f;
    prog_status_->operator()(0,1);
  }
  switch (std::get<0>(code)) {
  case OpeCode::ADD:
    qInfo() << "ADD";
    if (std::get<1>(code) == RegType::OUT) {
      register_->operator()(4, res);
    } else if (std::get<1>(code) == RegType::A) {
      register_->operator()(1, res);
    } else if (std::get<1>(code) == RegType::B) {
      register_->operator()(2, res);
    }
    prog_counter_->Increment();
    break;
  case OpeCode::MOV:
    qInfo() << "MOV";
    if (std::get<1>(code) == RegType::A) {
      register_->operator()(1, res);
    } else if (std::get<1>(code) == RegType::B) {
      register_->operator()(2, res);
    }
    prog_counter_->Increment();
    break;
  case OpeCode::IN:
    qInfo() << "IN";
    if (std::get<1>(code) == RegType::A) {
      register_->operator()(1, res);
    } else if (std::get<1>(code) == RegType::B) {
      register_->operator()(2, res);
    }
    prog_counter_->Increment();
    break;
  case OpeCode::OUT:
    qInfo() << "OUT";
    register_->operator()(4, res);
    prog_counter_->Increment();
    break;
  case OpeCode::JMP:
    qInfo() << "JMP";
    prog_counter_->operator()(res);
    qInfo() << "--> " << prog_counter_->operator()();
    break;
  case OpeCode::JNC:
    qInfo() << "JNC";
    if (!prog_status_->Read(0)) {
      prog_counter_->operator()(res);
    } else {
      prog_counter_->Increment();
    }
    prog_status_->operator()(0,0);
    break;
  }
  // execute
  if (prog_counter_->operator()() >= 15) {
    // end flag
    prog_counter_->operator()(0);
    return false;
  }
  // store result
  if (register_->Read(4)) {
    qInfo() << "**! LED !**";
  }
  register_->operator()(4,0);  // OUT port reset
  qInfo() << "C:[" << int(prog_status_->Read(0)) << "] A:[" << int(register_->Read(1)) << "] B:[" << int(register_->Read(2)) << "]";
  return true;
}

auto Core::Stop() -> void
{
  is_running_ = false;
  if (emu_thread_ != nullptr) {
    emu_thread_->RequestStop();
  }
  return;
}

}  // namespace CPU
