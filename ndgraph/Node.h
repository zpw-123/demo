//
// Created by Wang Allen on 2018/2/5.
//

#ifndef SKIPLISTPRO_NODE_H
#define SKIPLISTPRO_NODE_H

//forward declaration
template<typename K>
class SkipList;

template<typename K>
class Node {

    friend class SkipList<K>;

public:

    Node() {}

    Node(K k);

    ~Node();

    K getKey() const;

private:
    K key;
    Node<K> **forward;
    int nodeLevel;
};

template<typename K>
Node<K>::Node(const K k) {
    key = k;
};

template<typename K>
Node<K>::~Node() {
    delete[]forward;
};

template<typename K>
K Node<K>::getKey() const {
    return key;
}


#endif //SKIPLISTPRO_NODE_H
