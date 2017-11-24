#ifndef CMO_H
#define CMO_H

#include <stdint.h>

class ObIterator;
class ObRwIterator;
class NobArray;

struct CMO
{
  ObIterator init_ob_iterator(int32_t *data, int32_t len);
  ObRwIterator init_ob_rw_iterator(int32_t *data, int32_t len);
  NobArray init_nob_array(int32_t *data, int32_t len);
  void begin_leaky_sec();
  void end_leaky_sec();

  void begin_tx();
  void end_tx();
};

struct ObIterator
{
  int32_t read_next();
};

struct ObRwIterator
{
  int32_t read_next();
  void write_next(int32_t data);
};

struct NobArray
{
  int32_t read_at(int32_t addr);
  void write_at(int32_t addr, int32_t data);
};

#endif
