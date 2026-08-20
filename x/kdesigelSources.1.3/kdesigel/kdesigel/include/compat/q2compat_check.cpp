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
// Asserts must survive NDEBUG: this file IS the check.
#ifdef NDEBUG
#undef NDEBUG
#endif
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


    // --- behaviours corrected after review; previously untested -------------
    {   // Q2Queue::dequeue must unlink even when the head is null (qglist.cpp:623)
        Q2Queue<Thing> q; Thing a(1);
        q.enqueue(nullptr); q.enqueue(&a);
        int guard = 0;
        while (!q.isEmpty() && ++guard < 10) q.dequeue();
        assert(guard < 10);                       // used to spin forever
        assert(q.isEmpty());
    }
    {   // Q2Queue::remove exists in Qt 2 (qqueue.h:60) and MT_GUI calls it
        Q2Queue<Thing> q; Thing a(1), b(2);
        q.enqueue(&a); q.enqueue(&b);
        assert(q.remove()); assert(q.count() == 1); assert(q.head() == &b);
    }
    {   // Q2PtrList::remove(null): Qt 2 does not search, it removes current
        Thing a(1), b(2), c(3);
        Q2PtrList<Thing> l; l.append(&a); l.append(&b); l.append(&c);
        l.at(1);                                  // current = b
        const Thing *nothing = nullptr;
        assert(l.remove(nothing));
        assert(l.count() == 2 && l.getFirst() == &a && l.getLast() == &c);
    }
    {   // Q2ListIterator stays dead once off the end (qglist.cpp:1166)
        Thing a(1);
        Q2PtrList<Thing> l; l.append(&a);
        Q2ListIterator<Thing> it(l);
        assert(it.current() == &a);
        assert(it.operator++() == nullptr);
        assert(it.operator++() == nullptr);        // must not revive
        Q2PtrList<Thing> empty;
        Q2ListIterator<Thing> e(empty);
        assert(e.atFirst() && e.atLast());         // both true on empty in Qt 2
    }
    {   // Q2DictIterator::count() reads the live dict, not the snapshot
        Q2Dict<Thing> d; Thing a(1), b(2);
        d.insert("a", &a);
        Q2DictIterator<Thing> it(d);
        d.insert("b", &b);
        assert(it.count() == 2);
    }
    {   // Q2CString: Qt 2's buffer includes the terminating NUL
        Q2CString s("abc");
        assert(s.size() == 4 && s.count() == 4);
        assert(s[0] == 'a');
        // NOT s[s.size()-1]: size() counts the NUL as Qt 2 did, but operator[]
        // is QByteArray's and is bounds-checked against QByteArray::size().
        // Indexing the NUL asserts in a debug build. Documented in q2compat.h.
        assert(s.length() == 3);
        assert(s.contains('a') == 1);
        Q2CString n;
        assert(n.size() == 0);
        assert(static_cast<const char *>(n) == nullptr);
    }
    {   // out-of-range warns and clamps to index 0, as QGArray did
        Q2Array<int> a(3); a[0] = 10; a[1] = 11; a[2] = 12;
        assert(a.at(99) == 10);
        Thing t(1);
        Q2PtrVector<Thing> v(2); v.insert(0, &t);
        assert(v.at(99) == &t);
        Q2PtrVector<Thing> ve;
        assert(ve.at(0) == nullptr);
    }
    {   // copy ctor clears ownership; assignment keeps the destination's
        Q2PtrList<Thing> a; a.setAutoDelete(true);
        Q2PtrList<Thing> b; b.setAutoDelete(false);
        b = a;
        assert(!b.autoDelete());   // assignment keeps the DESTINATION's flag
        Q2Array<int> e;
        assert(static_cast<const int *>(e) == nullptr);   // null when empty
    }


    // --- owners must actually delete (M2: none of this was covered) ---------
    {   Q2PtrList<Thing> l; l.setAutoDelete(true);
        l.append(new Thing(1)); l.append(new Thing(2));
        assert(Thing::live == 2);
        l.remove(0u);              assert(Thing::live == 1);   // remove deletes
        Thing *t = l.take(0u);     assert(Thing::live == 1);   // take does NOT
        delete t;                  assert(Thing::live == 0);
        l.append(new Thing(3)); l.clear(); assert(Thing::live == 0); }  // clear deletes
    {   Q2PtrVector<Thing> v(3); v.setAutoDelete(true);
        v.insert(0, new Thing(1)); v.insert(1, new Thing(2));
        assert(Thing::live == 2);
        v.remove(0);               assert(Thing::live == 1);
        Thing *t = v.take(1);      assert(Thing::live == 1);
        delete t;
        v.insert(2, new Thing(4)); v.clear(); assert(Thing::live == 0); }
    {   Q2Dict<Thing> d; d.setAutoDelete(true);
        d.insert("a", new Thing(1)); d.insert("b", new Thing(2));
        assert(Thing::live == 2);
        d.remove("a");             assert(Thing::live == 1);
        Thing *t = d.take("b");    assert(Thing::live == 1);
        delete t;
        d.insert("c", new Thing(3)); d.clear(); assert(Thing::live == 0); }
    {   Q2Queue<Thing> q; q.setAutoDelete(true);
        q.enqueue(new Thing(1));
        Thing *t = q.dequeue();    assert(Thing::live == 1);   // dequeue must not delete
        delete t;
        q.enqueue(new Thing(2)); q.clear(); assert(Thing::live == 0); }
    assert(Thing::live == 0);
    {   // H1: Qt 2's QDict::operator[] is find() (qdict.h:67-68)
        Q2Dict<Thing> d; Thing a(7);
        d.insert("k", &a);
        assert(d["k"] == &a);
        assert(d["absent"] == nullptr); }

    std::printf("q2compat self-check: all assertions passed\n");
    return 0;
}
