#ifdef DEBUG
#include <iostream>
#endif /* DEBUG */

#include "thread_control_block.h"

ThreadControlBlock::ThreadControlBlock(uint8_t* stck, bool is_main)
  : m_is_sleeping(false)
  , m_state(READY)
  , m_stack(stck)
  , m_stack_alloc(false)
  , m_quantums(0)
  , m_sleep_quantums(0)
  , m_entry_point(nullptr)
{
  // if (m_stack == nullptr) {
  if (not is_main) {
    m_stack = new uint8_t[STACK_SIZE];
    m_stack_alloc = true;
  } else {
    m_quantums = 1;
  }
}

ThreadControlBlock::~ThreadControlBlock()
{
#ifdef DEBUG
  std::cout << "ThreadControlBlock: thread #" << m_tid << " destroied."
            << std::endl;
#endif /* DEBUG */

  if (m_stack_alloc) {
#ifdef DEBUG
    std::cout << "ThreadControlBlock: thread #" << m_tid << " delete stack"
              << std::endl;
#endif /* DEBUG */

    delete[] m_stack;
    m_stack = nullptr;
  }
}

int
ThreadControlBlock::get_quantums(void)
{
  return this->m_quantums;
}

const state&
ThreadControlBlock::get_state(void)
{
  return this->m_state;
}

void
ThreadControlBlock::set_state(const state& new_state)
{
  this->m_state = new_state;
}

void
ThreadControlBlock::sleep(int quantums)
{
  this->m_sleep_quantums = quantums;
}

void
ThreadControlBlock::update_quantums(int quantums)
{
  m_quantums += quantums;
  m_quantums_in_running += quantums;
  m_usec_in_running += quantums;
}

void
ThreadControlBlock::update_sleeping(int quantums)
{
  m_sleep_quantums += quantums;
}
int
ThreadControlBlock::get_remaining_sleep(void)
{
  return m_sleep_quantums;
}

int
ThreadControlBlock::get_quantums_in_running(void) const
{
  return m_quantums_in_running;
}

void
ThreadControlBlock::reset_quantums_in_running(void)
{
  m_quantums_in_running = 1;
}

int
ThreadControlBlock::get_usec_in_running(void) const
{
  return m_usec_in_running;
}

void
ThreadControlBlock::reset_usec_in_running(void)
{
  m_usec_in_running = 1;
}

uint8_t*
ThreadControlBlock::get_stack(void)
{
  return this->m_stack;
}

void
ThreadControlBlock::set_entry_point(thread_entry_point entry)
{
  m_entry_point = entry;
}

thread_entry_point
ThreadControlBlock::get_entry_point(void) const
{
  return m_entry_point;
}
