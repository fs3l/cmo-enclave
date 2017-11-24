#ifndef CODA_H
#define CODA_H

#include <stdint.h>

class ObIterator;
class ObRwIterator;
class NobArray;

class Coda
{
public:
  ObIterator init_ob_iterator(int32_t *data, int32_t len);
  ObRwIterator init_ob_rw_iterator(int32_t *data, int32_t len);
  NobArray init_nob_array(int32_t *data, int32_t len);
  void begin_leaky_sec();
  void end_leaky_sec();

  friend class ObIterator;
  friend class ObRwIterator;
  friend class NobArray;

private:
  void begin_tx();
  void end_tx();
};

class ObIterator
{
public:
  int32_t read_next();
};

class ObRwIterator
{
public:
  int32_t read_next();
  void write_next(int32_t data);
};

class NobArray
{
public:
  int32_t read_at(int32_t addr);
  void write_at(int32_t addr, int32_t data);
};

#endif
