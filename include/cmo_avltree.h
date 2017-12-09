#ifndef CMO_AVLTREE_H
#define CMO_AVLTREE_H

// Ref: http://www.geeksforgeeks.org/avl-tree-set-1-insertion/
//      http://www.geeksforgeeks.org/avl-tree-set-2-deletion/

#include "cmo.h"
#include "cmo_mempool.h"
#include "utils.h"

struct avl_tree_node {
  int16_t left_child;
  int16_t right_child;
  int32_t height;
};

typedef struct avl_tree_node avl_tree_node_t;

template <class Key>
struct avl_set_node {
  avl_tree_node meta;
  Key key;
};

template <class Key, class Value>
struct avl_map_node {
  avl_tree_node meta;
  Key key;
  Value value;
};

template <class Key, class Node>
class AVLTree
{
public:
  MemoryPool<Node> mem;
  int32_t* extra_data;
  NobArray_t* nob;
  CMO_p rt;

  AVLTree(CMO_p rt, int32_t capacity) : mem(rt, capacity)
  {
    extra_data = new int32_t[2];
    extra_data[0] = 0;  // root addr
    extra_data[1] = 0;  // num of element
    nob = init_nob_array(rt, extra_data, 2);
    this->rt = rt;
  }
  ~AVLTree() { delete[] extra_data; }
  void reset_nob(CMO_p rt)
  {
    mem.reset_nob(rt);
    nob = init_nob_array(rt, extra_data, 2);
    this->rt = rt;
  }
  void insert(const Node& node) { set_root(insert_at(get_root(), node)); }
  void remove(const Key& key) { set_root(remove_at(get_root(), key)); }
  int32_t find(const Key& key) const
  {
    int32_t addr = get_root();
    while (addr != 0) {
      Key _key = node_get_key(addr);
      if (key == _key) break;
      avl_tree_node_t meta = node_get_meta(addr);
      if (key < _key)
        addr = meta.left_child;
      else
        addr = meta.right_child;
    }
    return addr;
  }

  void get_node(int32_t addr, Node* node) const
  {
    mem.template read_block<Node>(addr, node);
  }
  void set_node(int32_t addr, const Node* node)
  {
    mem.template write_block<Node>(addr, node);
  }

  int32_t size() const { return nob_read_at(nob, 1); }
  int32_t begin_address() const { return mem.begin_address(); }
  int32_t next_address(int32_t addr) const { return mem.next_address(addr); }
private:
  int32_t get_root() const { return nob_read_at(nob, 0); }
  void set_root(int32_t new_root) { nob_write_at(nob, 0, new_root); }
  avl_tree_node_t node_get_meta(int32_t addr) const
  {
    avl_tree_node_t meta;
    mem.template read_block<avl_tree_node_t>(addr, &meta);
    return meta;
  }
  void node_set_meta(int32_t addr, const avl_tree_node_t* meta)
  {
    mem.template write_block<avl_tree_node_t>(addr, meta);
  }
  Key node_get_key(int32_t addr) const
  {
    Key key;
    // 2 is the offset of avl_tree_node_meta
    mem.template read_block<Key>(addr + 2, &key);
    return key;
  }
  int32_t node_height(int32_t addr) const
  {
    if (addr == 0) return 0;
    return node_get_meta(addr).height;
  }
  int32_t node_balance(int32_t addr) const
  {
    if (addr == 0) return 0;
    avl_tree_node_t meta = node_get_meta(addr);
    return node_height(meta.left_child) - node_height(meta.right_child);
  }
  int32_t find_min_key_node(int32_t addr) const
  {
    if (addr == 0) return addr;
    avl_tree_node_t meta = node_get_meta(addr);
    if (meta.left_child == 0)
      return addr;
    else
      return find_min_key_node(meta.left_child);
  }

  int32_t insert_at(int32_t addr, const Node& node)
  {
    if (addr == 0) {
      nob_write_at(nob, 1, size() + 1);
      Node _node = node;
      _node.meta.left_child = 0;
      _node.meta.right_child = 0;
      _node.meta.height = 1;
      if (mem.full()) cmo_abort(rt, "AVLTree: memory is full");
      int32_t _addr = mem.alloc_block();
      set_node(_addr, &_node);
      return _addr;
    }

    avl_tree_node_t meta = node_get_meta(addr);
    Key addr_key = node_get_key(addr);
    if (node.key < addr_key)
      meta.left_child = insert_at(meta.left_child, node);
    else if (node.key > addr_key)
      meta.right_child = insert_at(meta.right_child, node);
    else
      return addr;

    meta.height =
        1 + ::max(node_height(meta.left_child), node_height(meta.right_child));
    node_set_meta(addr, &meta);

    int32_t balance = node_balance(addr);

    if (balance > 1) {
      Key _l_key = node_get_key(meta.left_child);
      if (node.key < _l_key) return right_rotate(addr);
      if (node.key > _l_key) {
        meta.left_child = left_rotate(meta.left_child);
        node_set_meta(addr, &meta);
        return right_rotate(addr);
      }
    } else if (balance < -1) {
      Key _r_key = node_get_key(meta.right_child);
      if (node.key > _r_key) return left_rotate(addr);
      if (node.key < _r_key) {
        meta.right_child = right_rotate(meta.right_child);
        node_set_meta(addr, &meta);
        return left_rotate(addr);
      }
    }

    return addr;
  }
  int32_t remove_at(int32_t addr, const Key& key)
  {
    if (addr == 0) return addr;

    avl_tree_node_t meta = node_get_meta(addr);
    Key _key = node_get_key(addr);
    if (key < _key) {
      meta.left_child = remove_at(meta.left_child, key);
      node_set_meta(addr, &meta);
    } else if (key > _key) {
      meta.right_child = remove_at(meta.right_child, key);
      node_set_meta(addr, &meta);
    } else {
      if (meta.left_child == 0) {
        nob_write_at(nob, 1, size() - 1);
        mem.free_block(addr);
        return meta.right_child;
      } else if (meta.right_child == 0) {
        nob_write_at(nob, 1, size() - 1);
        mem.free_block(addr);
        return meta.left_child;
      }
      int32_t _addr = find_min_key_node(meta.right_child);
      Node _node;
      get_node(_addr, &_node);
      _node.meta = meta;
      _node.meta.right_child = remove_at(_node.meta.right_child, _node.key);
      set_node(addr, &_node);
    }

    if (addr == 0) return addr;

    meta = node_get_meta(addr);
    meta.height =
        1 + ::max(node_height(meta.left_child), node_height(meta.right_child));

    node_set_meta(addr, &meta);

    int32_t balance = node_balance(addr);

    if (balance > 1) {
      int32_t left_balance = node_balance(meta.left_child);
      if (left_balance >= 0) return right_rotate(addr);
      if (left_balance < 0) {
        meta.left_child = left_rotate(meta.left_child);
        node_set_meta(addr, &meta);
        return right_rotate(addr);
      }
    } else if (balance < -1) {
      int32_t right_balance = node_balance(meta.right_child);
      if (right_balance <= 0) return left_rotate(addr);
      if (right_balance > 0) {
        meta.right_child = right_rotate(meta.right_child);
        node_set_meta(addr, &meta);
        return left_rotate(addr);
      }
    }

    return addr;
  }
  int32_t left_rotate(int32_t addr)
  {
    avl_tree_node_t meta = node_get_meta(addr);
    int32_t right_addr = meta.right_child;
    avl_tree_node_t right_meta = node_get_meta(right_addr);
    int32_t tmp = right_meta.left_child;
    right_meta.left_child = addr;
    meta.right_child = tmp;
    meta.height =
        1 + ::max(node_height(meta.left_child), node_height(meta.right_child));
    right_meta.height =
        1 + ::max(meta.height, node_height(right_meta.right_child));
    node_set_meta(addr, &meta);
    node_set_meta(right_addr, &right_meta);
    return right_addr;
  }
  int32_t right_rotate(int32_t addr)
  {
    avl_tree_node_t meta = node_get_meta(addr);
    int32_t left_addr = meta.left_child;
    avl_tree_node_t left_meta = node_get_meta(left_addr);
    int32_t tmp = left_meta.right_child;
    left_meta.right_child = addr;
    meta.left_child = tmp;
    meta.height =
        1 + ::max(node_height(meta.left_child), node_height(meta.right_child));
    left_meta.height =
        1 + ::max(node_height(left_meta.left_child), meta.height);
    node_set_meta(addr, &meta);
    node_set_meta(left_addr, &left_meta);
    return left_addr;
  }

  void print() const
  {
    print_message("--------\n");
    print_at(get_root(), 0);
    if (!is_balance()) print_message("unbalance!!\n");
  }
  void print_at(int32_t addr, int32_t indent) const
  {
    if (addr == 0) return;
    print_message("%*s", indent, "");
    Node n;
    get_node(addr, &n);
    print_message("node[addr=%d, key=%d, height=%d]\n", addr, n.key,
                  n.meta.height);
    print_at(n.meta.left_child, indent + 2);
    print_at(n.meta.right_child, indent + 2);
  }
  bool is_balance() const { return is_balance_at(get_root()); }
  bool is_balance_at(int32_t addr) const
  {
    if (addr == 0) return true;
    int32_t balance = node_balance(addr);
    avl_tree_node_t meta = node_get_meta(addr);
    return (balance == -1 || balance == 0 || balance == 1) &&
           is_balance_at(meta.left_child) && is_balance_at(meta.right_child);
  }
};

template <class T>
class AVLSet
{
public:
  AVLSet(CMO_p rt, int32_t capacity) : tree(rt, capacity) {}
  ~AVLSet() {}
  void reset_nob(CMO_p rt) { tree.reset_nob(rt); }
  void insert(const T& element)
  {
    struct avl_set_node<T> node;
    node.key = element;
    tree.insert(node);
  }
  void remove(const T& element) { tree.remove(element); }
  bool find(const T& element) const { return tree.find(element) != 0; }
  int32_t size() const { return tree.size(); }
  void get_element(int32_t addr, T* element) const
  {
    struct avl_set_node<T> node;
    tree.get_node(addr, &node);
    *element = node.key;
  }
  int32_t begin_address() const { return tree.begin_address(); }
  int32_t next_address(int32_t addr) const { return tree.next_address(addr); }
private:
  AVLTree<T, struct avl_set_node<T>> tree;
};

template <class K, class V>
class AVLMap
{
public:
  AVLMap(CMO_p rt, int32_t capacity) : tree(rt, capacity) {}
  ~AVLMap() {}
  void reset_nob(CMO_p rt) { tree.reset_nob(rt); }
  void insert(const K& key, const V& value)
  {
    struct avl_map_node<K, V> node;
    node.key = key;
    node.value = value;
    tree.insert(node);
  }
  void remove(const K& key) { tree.remove(key); }
  int32_t find(const K& key) const { return tree.find(key); }
  int32_t size() const { return tree.size(); }
  void get_element(int32_t addr, K* key, V* value) const
  {
    struct avl_map_node<K, V> node;
    tree.get_node(addr, &node);
    *key = node.key;
    *value = node.value;
  }
  void get_key(int32_t addr, K* key) const
  {
    struct avl_map_node<K, V> node;
    tree.get_node(addr, &node);
    *key = node.key;
  }
  void get_value(int32_t addr, V* value) const
  {
    struct avl_map_node<K, V> node;
    tree.get_node(addr, &node);
    *value = node.value;
  }
  void set_value(int32_t addr, const V& value)
  {
    struct avl_map_node<K, V> node;
    tree.get_node(addr, &node);
    node.value = value;
    tree.set_node(addr, &node);
  }
  int32_t begin_address() const { return tree.begin_address(); }
  int32_t next_address(int32_t addr) const { return tree.next_address(addr); }
private:
  AVLTree<K, struct avl_map_node<K, V>> tree;
};

#endif
