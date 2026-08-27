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
#include <QStringList>
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
    {   // Q2PtrList::remove(ptr) removes THAT pointer, reports whether it
        // unlinked anything, and never frees while the list does not own.
        // SIGEL_MasterGUI/SIG_GPParameter.cpp deletes on a true return, so
        // all three properties are load-bearing.
        Thing a(1), b(2), absent(3);
        Q2PtrList<Thing> l; l.append(&a); l.append(&b);
        l.first();                                // current = a
        assert(l.remove(&b));                     // removes b, not current
        assert(l.count() == 1 && l.getFirst() == &a);
        assert(!l.remove(&absent));               // absent: false, no removal
        assert(l.count() == 1);
        // A failed search goes through find(), which kills the cursor
        // (qglist.cpp:683-726): at() == -1 and current() == 0 afterwards.
        assert(l.at() == -1 && l.current() == nullptr);
        assert(!l.remove(&b));                    // already gone: false
        assert(l.count() == 1);
        assert(Thing::live == 3);                 // does not own: freed nothing
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


    {   // Q2ValueList iterators must survive an append, as Qt 2's linked list did.
        // SIG_GPManager.cpp:106-217 appends while iterating and then passes the
        // iterator to remove(); on a contiguous QList that is a use-after-free.
        Q2ValueList<int> v;
        for (int i = 0; i < 4; ++i) v << i;
        Q2ValueList<int>::Iterator it = v.begin();
        ++it;                                   // points at 1
        for (int i = 0; i < 2000; ++i) v << i;  // forces any reallocation
        assert(*it == 1);                       // iterator still valid
        it = v.remove(it);                      // Qt 2 returns the next one
        assert(*it == 2);
        assert(v.count() == 2003);
        assert(v.contains(0) == 2);             // Qt 2 returns a count
    }


    {   // PHASE D: iteration order is INSERTION order. This assertion used to
        // pin Qt 2's hash order for the real insect robot -- the order that
        // numbered links in SIG_DynaMoSimulationData.cpp:33-51. That order now
        // comes from the data files instead, so what has to hold here is that
        // the container itself reorders nothing.
        const char *ins[] = { "body","leg1","foot1","leg2","foot2","leg3","foot3",
                              "leg4","foot4","leg5","foot5","leg6","foot6" };
        Thing store[13];
        Q2Dict<Thing> d;
        for (int i = 0; i < 13; ++i) d.insert(QString::fromLatin1(ins[i]), &store[i]);
        int n = 0;
        for (Q2DictIterator<Thing> it(d); it.current(); ++it, ++n) {
            assert(it.currentKey() == QLatin1String(ins[n]));
            assert(it.current() == &store[n]);
        }
        assert(n == 13);
        assert(d.count() == 13);
        assert(d.size() == 17);          // table size, not item count
    }
    {   // duplicate keys: newest wins, remove() takes one (qgdict.cpp:379-386)
        Q2Dict<Thing> d(31); Thing a(1), b(2), c(3);
        d.insert("k", &a); d.insert("k", &b); d.insert("k", &c);
        assert(d.count() == 3 && d.find("k") == &c);
        d.remove("k");
        assert(d.count() == 2 && d.find("k") == &b);
    }


    {   // B1: an owner that frees explicitly must free exactly once, and a
        // dictionary with no flag must free nothing.
        Q2Dict<Thing> owned;
        owned.insert("a", new Thing(1));
        owned.insert("b", new Thing(2));
        assert(Thing::live == 2);
        owned.deleteContents();
        assert(Thing::live == 0);
        assert(owned.count() == 0);
        owned.deleteContents();               // idempotent: no double free
        assert(Thing::live == 0);
    }
    {   Thing a(1), b(2);
        Q2Dict<Thing> observing;              // no flag, no deleteContents call
        observing.insert("a", &a); observing.insert("b", &b);
    }                                          // must not free a or b
    assert(Thing::live == 0);


    {   // B3: Q2PtrList owners. Every list converted in B3 frees with
        // deleteContents() both mid-life (where 2003 called clear()) and in
        // its owner's destructor.
        Q2PtrList<Thing> owned;
        owned.append(new Thing(1));
        owned.append(new Thing(2));
        assert(Thing::live == 2);
        owned.deleteContents();               // stands in for the old clear()
        assert(Thing::live == 0 && owned.count() == 0);
        owned.append(new Thing(3));
        assert(Thing::live == 1);
        owned.deleteContents();               // stands in for the destructor
        assert(Thing::live == 0);
        owned.deleteContents();               // idempotent
        assert(Thing::live == 0);
    }
    {   Thing a(1), b(2);
        Q2PtrList<Thing> observing;           // no flag, no deleteContents call
        observing.append(&a); observing.append(&b);
    }                                          // must not free a or b
    assert(Thing::live == 0);
    {   // The mid-life free path a converted list still needs: 2003's
        // remove() on an owning list was a delete.
        Q2PtrList<Thing> owned;
        owned.append(new Thing(1));
        owned.append(new Thing(2));
        owned.first();
        delete owned.take();                  // take() at the cursor
        assert(Thing::live == 1 && owned.count() == 1);
        owned.deleteContents();
        assert(Thing::live == 0);
    }

    {   // B4: Q2PtrVector owners. insert() over an occupied slot was a delete
        // under setAutoDelete, so a converted caller must free the slot first.
        Q2PtrVector<Thing> owned(2);
        owned.insert(0, new Thing(1));
        owned.insert(1, new Thing(2));
        assert(Thing::live == 2);
        delete owned.take(0);                 // the free that insert() did
        owned.insert(0, new Thing(3));
        assert(Thing::live == 2);
        owned.deleteContents();
        assert(Thing::live == 0);
        assert(owned.size() == 2);            // slots kept, callers index them
        owned.deleteContents();               // idempotent
        assert(Thing::live == 0);
    }
    {   // A Q2PtrVector normally has null holes -- dynaMechsLinks and tours
        // both do -- so deleteContents() must walk size(), the allocated
        // slots, not count(), the occupied ones.
        Q2PtrVector<Thing> holed(3);
        holed.insert(0, new Thing(1));
        holed.insert(2, new Thing(2));        // slot 1 stays null
        assert(Thing::live == 2 && holed.size() == 3 && holed.count() == 2);
        holed.deleteContents();
        assert(Thing::live == 0);
    }
    {   Thing a(1), b(2);
        Q2PtrVector<Thing> observing(2);      // no flag, no deleteContents call
        observing.insert(0, &a); observing.insert(1, &b);
    }                                          // must not free a or b
    assert(Thing::live == 0);

    {   // deleteContents() must free EVERY entry of a bucket, not just the
        // head or the tail. ADD, DELAY and MIN all hash to bucket 14 at the
        // default vlen of 17 -- that is the real chain in the default
        // SIG_LanguageParameters dictionary.
        Q2Dict<Thing> d;
        d.insert("ADD", new Thing(1));
        d.insert("DELAY", new Thing(2));
        d.insert("MIN", new Thing(3));
        d.insert("MAX", new Thing(4));        // bucket 15, on its own
        assert(Thing::live == 4);
        d.deleteContents();
        assert(Thing::live == 0 && d.count() == 0);
    }
    {   // take() must drop the item count. SIG_LanguageParameters writes
        // count() as a record count and the reader consumes exactly that
        // many, so a stale count corrupts every saved experiment.
        Q2Dict<Thing> d;
        d.insert("ADD", new Thing(1));
        d.insert("DELAY", new Thing(2));
        d.insert("MIN", new Thing(3));
        assert(d.count() == 3);
        delete d.take("DELAY");               // from the middle of a chain
        assert(d.count() == 2 && Thing::live == 2);
        assert(d.find("ADD") && d.find("MIN") && !d.find("DELAY"));
        d.deleteContents();
        assert(Thing::live == 0);
    }

    {   // clear() on a container that does NOT own must free nothing.
        // SIG_Robot.cpp hand-deletes six dictionaries' contents and then
        // calls clear() on each; giving clear() teeth is six double frees.
        Thing a(1), b(2), c(3);
        Q2Dict<Thing> d;      d.insert("ADD", &a); d.insert("DELAY", &b);
        Q2PtrList<Thing> l;   l.append(&a); l.append(&b);
        Q2PtrVector<Thing> v(2); v.insert(0, &a); v.insert(1, &b);
        d.clear(); l.clear(); v.clear();
        assert(Thing::live == 3);
        assert(d.count() == 0 && l.count() == 0 && v.count() == 0);
    }
    {   // Same for the vector paths where Qt 2's flag was the delete:
        // insert() over an occupied slot, remove(), and a shrinking resize().
        Thing a(1), b(2), c(3);
        Q2PtrVector<Thing> v(3);
        v.insert(0, &a); v.insert(1, &b); v.insert(2, &c);
        v.insert(0, &c);                      // overwrite an occupied slot
        v.remove(1);
        v.resize(1);                          // truncate the tail
        assert(Thing::live == 3);
    }

    {   // B: an owner must free on EVERY path, not only in its destructor.
        // 2003 armed setAutoDelete in the constructor, so remove() freed too;
        // a destructor-only conversion silently loses that.
        Q2Dict<Thing> d;
        d.insert("a", new Thing(1));
        d.insert("b", new Thing(2));
        assert(Thing::live == 2);
        delete d.take("a");                 // the mid-life free path
        assert(Thing::live == 1);
        d.deleteContents();
        assert(Thing::live == 0);
    }
    {   // PHASE D: resize() no longer places anything -- there is one chain --
        // so it must leave insertion order alone. It used to have to reproduce
        // Qt 2's rehash, which yielded k7,k4,k1.
        Q2Dict<Thing> d(17); Thing a(1), b(2), c(3);
        d.insert("k1", &a); d.insert("k4", &b); d.insert("k7", &c);
        d.resize(3);
        QStringList got;
        for (Q2DictIterator<Thing> it(d); it.current(); ++it) got << it.currentKey();
        assert(got.join(',') == QLatin1String("k1,k4,k7"));
        assert(d.count() == 3 && d.find("k4") == &b);
    }
    {   // assignment keeps the DESTINATION's table size (qgdict.cpp:280-302)
        Q2Dict<Thing> src(31), dst(17); Thing a(1);
        src.insert("x", &a);
        dst = src;
        assert(dst.size() == 17 && dst.count() == 1 && dst.find("x") == &a);
    }

    std::printf("q2compat self-check: all assertions passed\n");
    return 0;
}
