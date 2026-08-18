// Exit criterion for step A0 (PORTING.md D11).
//
// Two jobs:
//  1. Explicit instantiation, so -fsyntax-only actually checks the template
//     bodies.  Without this the criterion is vacuous for a header nothing
//     includes yet.  From A1 on, real sources instantiate through use.
//  2. An assert-based self-check of the ownership and cursor semantics, which
//     no compile-time check can verify.  The A0 review found a double-free in
//     the copy constructors that compiled perfectly.
//
//   compile: g++ -fsyntax-only -std=c++17 -Wall -Wextra -I<inc> -I<qt6> this
//   run:     g++ -std=c++17 -I<inc> -I<qt6> this -lQt6Core -o chk && ./chk

#include "compat/q2compat.h"
#include <cassert>
#include <cstdio>

struct Thing {
    int v;
    static int live;
    explicit Thing(int x = 0) : v(x) { ++live; }
    Thing(const Thing &o) : v(o.v) { ++live; }
    ~Thing() { --live; }
    bool operator<(const Thing &o) const { return v < o.v; }
};
int Thing::live = 0;

template class Q2Array<double>;
template class Q2Array<int>;
template class Q2Dict<Thing>;
template class Q2DictIterator<Thing>;
template class Q2PtrVector<Thing>;
template class Q2PtrList<Thing>;
template class Q2ListIterator<Thing>;
template class Q2Queue<Thing>;
template class Q2ValueList<int>;

int main()
{
    // --- qcollection.h:64 -- a Qt 2 copy is ALWAYS non-owning ---------------
    {
        Q2PtrList<Thing> a;
        a.setAutoDelete(true);
        a.append(new Thing(1));
        {
            Q2PtrList<Thing> b(a);              // must NOT inherit ownership
            assert(!b.autoDelete());
        }                                        // b dies: must not free a's item
        assert(Thing::live == 1);
    }                                            // a dies: frees exactly once
    assert(Thing::live == 0);

    { Q2PtrVector<Thing> a(1); a.setAutoDelete(true); a.insert(0, new Thing(1));
      { Q2PtrVector<Thing> b(a); assert(!b.autoDelete()); }
      assert(Thing::live == 1); }
    assert(Thing::live == 0);

    { Q2Dict<Thing> a; a.setAutoDelete(true); a.insert("k", new Thing(1));
      { Q2Dict<Thing> b(a); assert(!b.autoDelete()); }
      assert(Thing::live == 1); }
    assert(Thing::live == 0);

    // --- qglist.cpp:364-376, 436-473 -- cursor placement --------------------
    {
        Thing t0(0), t1(1), t2(2), t3(3);
        Q2PtrList<Thing> l;
        l.append(&t0); l.append(&t1); l.append(&t2); l.append(&t3);
        assert(l.at() == 3);                     // append -> new last index
        l.prepend(&t0); assert(l.at() == 0);
        l.remove(0u);

        l.at(3); l.remove(1u);   assert(l.at() == 1);   // survivor slid in
        l.append(&t3);
        l.at(3); l.removeFirst(); assert(l.at() == 0);
        l.at(0); l.removeLast();  assert(l.at() == int(l.count()) - 1);
    }

    // --- qgvector.cpp:262-265, 338-352 -- insert/resize delete --------------
    {
        Q2PtrVector<Thing> v(2);
        v.setAutoDelete(true);
        v.insert(0, new Thing(1));
        v.insert(0, new Thing(2));               // must delete the first
        assert(Thing::live == 1);
        assert(v.size() == 2 && v.count() == 1); // slots vs occupants
        v.resize(0);                             // must delete the truncated one
        assert(Thing::live == 0);
    }

    // --- qgdict.cpp:379-386 -- duplicate keys stack newest-first ------------
    {
        Q2Dict<Thing> d(31);
        Thing a(1), b(2), c(3);
        d.insert("k", &a); d.insert("k", &b); d.insert("k", &c);
        assert(d.count() == 3);                  // items
        assert(d.size() == 31);                  // buckets, NOT items
        assert(d.find("k") == &c);               // newest
        d.remove("k");
        assert(d.count() == 2);                  // one, not all
    }

    // --- divergence 1 -- numeric sort, not memcmp byte order ----------------
    {
        Q2Array<int> a(4);
        a[0] = 300; a[1] = 100; a[2] = 2; a[3] = 1;
        a.sort();
        assert(a[0] == 1 && a[1] == 2 && a[2] == 100 && a[3] == 300);
    }

    std::printf("q2compat self-check: all assertions passed\n");
    return 0;
}
