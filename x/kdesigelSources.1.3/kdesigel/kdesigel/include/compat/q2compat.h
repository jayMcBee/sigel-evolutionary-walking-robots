#ifndef Q2COMPAT_H
#define Q2COMPAT_H

// Qt 2.3 -> Qt 6 compatibility shim.  PORTING.md D1(a), step A0.
//
// Reproduces Qt 2.3 container semantics on top of Qt 6 so that steps A1-A9 are
// pure renames and every ownership decision defers to Phase B. Phase D deletes
// this file.
//
//   QCString    -> Q2CString        QQueue      -> Q2Queue
//   QValueList  -> Q2ValueList      + matching iterators
//
// The trap this defuses: Qt 2's QVector and QList hold *pointers*, Qt 6's hold
// *values*, and the names are identical.  A naive rename compiles clean and
// then double-frees.
//
// Every semantic below was read out of the vendored Qt 2.3 sources under
// x/supportingLibs/supportingLibs/qt/src/tools/ rather than recalled.
//
// DELIBERATE DIVERGENCES FROM Qt 2 -- see PORTING.md section 9:
//
//  1. Out-of-range indexing warns and clamps to index 0, as Qt 2's QGArray::at
//     did (qgarray.h:108-117). Qt 2's QGVector::at only warned and then read out
//     of range anyway (qgvector.h:85-92); clamping is used for both. This is
//     done here rather than left to Q_ASSERT, which compiles to nothing under
//     QT_NO_DEBUG and would make a release build corrupt memory silently where
//     2003 merely returned a wrong value. Two cases: clamp to 0, or null for an
//     empty Q2PtrVector, where null is a normal slot.
//
//  The numeric-sort and resize()/Q2Array(int) divergences were about Q2Array,
//  deleted in Phase D. Its two sort callers are now std::sort directly;
//  SIG_AllIndividualsView.cpp:240 still uses a raw Qt 2 QArray and will fail
//  loudly in Phase C. The Q2Dict copy divergence went with Q2Dict.
//
//  Q2ValueList::contains DOES reproduce Qt 2, returning an occurrence count
//  rather than Qt 6's bool (qvaluelist.h:260-268). An earlier version of this
//  note claimed the opposite.
//
//  Four smaller divergences are known and left as they are. All four were
//  confirmed by running the same operations against the vendored Qt 2.3
//  sources, and NO SIGEL caller reaches any of them. Listed so that a future
//  conversion which does reach one finds it recorded rather than by surprise:
//
//    - An out-of-range at()/insert()/remove()/take() on a Q2PtrList leaves the
//      cursor dead. Qt 2's locate() revives it at element 0 before the range
//      check (qglist.cpp:275-292). No SIGEL site indexes a list out of range.
//    - Q2CString::resize(0) leaves a non-null string; Qt 2 freed the buffer and
//      the string became null (qcstring.cpp:568-583). All nine Q2CString sites
//      only take a const char* out of a toUtf8().
//    - Q2Array::data() is non-null after resize(0); Qt 2 returned 0
//      (qgarray.cpp:213-216). isNull() and operator const T* still agree with
//      Qt 2. All 14 data() sites are on locals sized immediately before.
//    - Q2PtrVector's insert/remove/take are silent when the index is out of
//      range; Qt 2 emitted a qWarning (qgvector.cpp:254-314). Return values and
//      resulting state are identical.
//
//  4. Q2Array is copy-on-write; Qt 2's QArray is TRULY shared, with no COW at
//     all (qgarray.cpp:134-138, 284-294).  In Qt 2, `b = a; b.at(0) = 9;` is
//     visible through `a`, and data() hands out the shared buffer.  Under this
//     shim each holder gets its own copy on first write.  at()/operator[]/data()
//     are declared const but call non-const QList members, so even a READ
//     detaches where Qt 2 never did.  Verified safe today: of the 14 .data()
//     sites at least eight write through the pointer, but each does so while the
//     refcount is 1, and no raw pointer is held across a copy.

#include <QByteArray>
#include <QDebug>
#include <QHashSeed>
#include <QList>
#include <QString>
#include <list>

// ---------------------------------------------------------------------------
// Q2Dict reproduced Qt 2's own hash table (deleted in Phase D), so its order no longer depends on
// QHash at all. This remains for every OTHER QHash in the process: Qt 6
// randomises the seed once per run, and SIGEL is expected to reproduce a run
// from a fixed RANDOMSEED.
//
// One object per translation unit; the call is idempotent. main() should call
// QHashSeed::setDeterministicGlobalSeed() explicitly as well, once sigel.cpp and
// sigel_slave.cpp are ported -- static initialisation order is unspecified, so
// this cannot protect a QHash built by another static constructor.
namespace {
struct Q2DeterministicHashSeed {
    Q2DeterministicHashSeed() { QHashSeed::setDeterministicGlobalSeed(); }
};
const Q2DeterministicHashSeed q2DeterministicHashSeedInit;
}
// Q2Array was here. DELETED in Phase D, 2026-08-27: it was a value array over
// QList all along, so its 180 sites are now plain QList. Two behaviours went
// with it -- sort() was numeric (D13) and is now std::sort at the two sites
// that used it, and at() clamped an out-of-range index instead of failing.
// The clamp fired in none of 42 evaluations, and both defects it was masking
// (SIG_ProgramLine.cpp:215, SIG_DynaSystem.cpp:266) were already fixed.

// ---------------------------------------------------------------------------
// Q2Dict<T>  <- Qt 2 QDict<T>: QString-keyed dictionary of *pointers*.
//
// Q2Dict and Q2DictIterator were here. DELETED in Phase D, 2026-08-27: the
// ordering they carried now lives in the data files, and their eight users are
// plain QList -- SIG_Robot's six, SIG_Link::points and
// SIG_LanguageParameters::allowedCommands. See PORTING.md §10.

// ---------------------------------------------------------------------------
// Q2PtrVector<T>  <- Qt 2 QVector<T>: fixed-length array of pointers with null
// holes.  qgvector.h/.cpp:
//   size()  == allocated slots (len)      count() == occupied slots (numItems)
//   insert(i,d) DELETES slot i's existing occupant (qgvector.cpp:262-265) and
//               does not shift; i >= size() is a range error, not a grow
//   resize(n) shrinking DELETES every truncated item (qgvector.cpp:338-352)
// ---------------------------------------------------------------------------
template <class T>
class Q2PtrVector
{
public:
    Q2PtrVector() {}
    explicit Q2PtrVector(uint size) { v.assign(qsizetype(size), nullptr); }

    Q2PtrVector(const Q2PtrVector &o) : v(o.v), del(false) {}   // qcollection.h:64
    Q2PtrVector &operator=(const Q2PtrVector &o)
    {
        if (this != &o) { clear(); v = o.v; }                   // dest keeps its own del
        return *this;
    }

    ~Q2PtrVector() { if (del) deleteAll(); }

    uint size() const { return uint(v.size()); }

    // ponytail: O(n) scan rather than a maintained counter -- cannot fall out of
    // sync, and this class dies in Phase D.  Two hot spots if that ever matters:
    // SIG_Geometry.cpp:49,53 use it as a re-evaluated loop bound, making model
    // load O(n^2) over VRML meshes.
    uint count() const
    {
        uint n = 0;
        for (T *p : v)
            if (p)
                ++n;
        return n;
    }

    bool isEmpty() const { return count() == 0; }
    bool isNull() const { return v.isEmpty(); }

    void setAutoDelete(bool enable) { del = enable; }
    bool autoDelete() const { return del; }

    bool resize(uint newsize)
    {
        const qsizetype n = qsizetype(newsize);
        if (n < v.size() && del)
            for (qsizetype k = n; k < v.size(); ++k)
                delete v.at(k);
        v.resize(n);
        return true;
    }

    bool insert(uint i, const T *d)
    {
        if (qsizetype(i) >= v.size())
            return false;                          // Qt 2: range error, no grow
        if (v.at(qsizetype(i)) && del)
            delete v.at(qsizetype(i));             // Qt 2: old occupant deleted
        v[qsizetype(i)] = const_cast<T *>(d);
        return true;
    }

    bool remove(uint i)
    {
        if (qsizetype(i) >= v.size())
            return false;
        if (v.at(qsizetype(i)) && del)
            delete v.at(qsizetype(i));
        v[qsizetype(i)] = nullptr;                 // slot stays, hole appears
        return true;
    }

    T *take(uint i)
    {
        if (qsizetype(i) >= v.size())
            return nullptr;
        T *p = v.at(qsizetype(i));
        v[qsizetype(i)] = nullptr;                 // never deletes
        return p;
    }

    void clear() { if (del) deleteAll(); v.clear(); }

    // Phase B: the owner frees its items explicitly. Slots stay, as clear()
    // does not -- callers index this container by slot number.
    void deleteContents()
    {
        for (qsizetype i = 0; i < v.size(); ++i) { delete v.at(i); v[i] = nullptr; }
    }

    // As Q2Array::at above: warn and clamp instead of relying on Q_ASSERT.
    // A null slot is already normal for this container, so an empty vector
    // yields null -- there is no element to clamp to.
    T *at(uint i) const
    {
        if (qsizetype(i) < v.size())
            return v.at(qsizetype(i));
        qWarning("Q2PtrVector::at: index %u out of range (size %lld)",
                 i, static_cast<long long>(v.size()));
        return v.isEmpty() ? nullptr : v.at(0);
    }
    T *operator[](int i) const { return at(uint(i)); }

    int find(const T *d, uint i = 0) const { return int(v.indexOf(const_cast<T *>(d), qsizetype(i))); }
    int findRef(const T *d, uint i = 0) const { return find(d, i); }
    uint contains(const T *d) const { return uint(v.count(const_cast<T *>(d))); }
    uint containsRef(const T *d) const { return contains(d); }

private:
    void deleteAll() { for (T *p : v) delete p; }
    QList<T *> v;
    bool del = false;
};

// ---------------------------------------------------------------------------
// Q2PtrList<T>  <- Qt 2 QList<T>: pointer list with an internal cursor.
// qglist.cpp: first/last/next/prev/at(i)/find all move the cursor; append sets
// it to the new last index (:373), prepend to 0, insertAt to the insert index.
// removeAt/takeAt locate(i) FIRST, so the cursor lands on the removal site and
// unlink then leaves it on whatever slid in (:436-473).
// ---------------------------------------------------------------------------
template <class T>
class Q2PtrList
{
public:
    Q2PtrList() {}

    // qglist.cpp:183 -- QCollection(list) clears del_item, then append() per
    // item leaves the cursor on the last one.
    Q2PtrList(const Q2PtrList &o) : v(o.v), del(false) { cur = v.isEmpty() ? -1 : v.size() - 1; }

    // qglist.cpp:222-235 -- clear() first, copy, cursor to 0, dest flag kept.
    Q2PtrList &operator=(const Q2PtrList &o)
    {
        if (this != &o) { clear(); v = o.v; cur = v.isEmpty() ? -1 : 0; }
        return *this;
    }

    ~Q2PtrList() { if (del) qDeleteAll(v); }

    uint count() const { return uint(v.size()); }
    bool isEmpty() const { return v.isEmpty(); }

    void setAutoDelete(bool enable) { del = enable; }
    bool autoDelete() const { return del; }

    void append(const T *d) { v.append(const_cast<T *>(d)); cur = v.size() - 1; }
    void prepend(const T *d) { v.prepend(const_cast<T *>(d)); cur = 0; }

    bool insert(uint i, const T *d)
    {
        if (qsizetype(i) > v.size())
            return false;
        v.insert(qsizetype(i), const_cast<T *>(d));
        cur = qsizetype(i);
        return true;
    }

    T *at(uint i)
    {
        if (qsizetype(i) >= v.size())
            return nullptr;
        cur = qsizetype(i);
        return v.at(cur);
    }
    int at() const { return int(cur); }        // Qt 2: the current *index*

    T *current() const { return valid() ? v.at(cur) : nullptr; }
    T *getFirst() const { return v.isEmpty() ? nullptr : v.first(); }
    T *getLast() const { return v.isEmpty() ? nullptr : v.last(); }

    T *first() { cur = v.isEmpty() ? -1 : 0; return current(); }
    T *last() { cur = v.size() - 1; return current(); }

    T *next()
    {
        if (cur < 0)
            return nullptr;
        if (++cur >= v.size()) { cur = -1; return nullptr; }
        return v.at(cur);
    }

    T *prev()
    {
        if (cur <= 0) { cur = -1; return nullptr; }
        --cur;
        return v.at(cur);
    }

    // Qt 2's default compareItems() is pointer identity (qglist.cpp:125-128),
    // so find == findRef.  Nothing in SIGEL overrides it.
    int find(const T *d) { cur = v.indexOf(const_cast<T *>(d)); return int(cur); }
    int findRef(const T *d) { return find(d); }
    uint contains(const T *d) const { return uint(v.count(const_cast<T *>(d))); }
    uint containsRef(const T *d) const { return contains(d); }

    bool remove(uint i)
    {
        if (qsizetype(i) >= v.size())
            return false;
        if (del)
            delete v.at(qsizetype(i));
        v.removeAt(qsizetype(i));
        cursorAfterRemoval(qsizetype(i));
        return true;
    }
    bool remove() { return valid() ? remove(uint(cur)) : false; }
    // qglist.cpp:504-516 -- a null argument does NOT search; it removes
    // current. A failed search goes through find(), which kills the cursor
    // (qglist.cpp:683-726), so at() == -1 and current() == 0 afterwards.
    bool remove(const T *d)
    {
        if (!d)
            return remove();
        const qsizetype i = v.indexOf(const_cast<T *>(d));
        if (i < 0) { cur = -1; return false; }
        return remove(uint(i));
    }
    bool removeRef(const T *d) { return remove(d); }
    bool removeFirst() { return v.isEmpty() ? false : remove(0u); }
    bool removeLast() { return v.isEmpty() ? false : remove(uint(v.size() - 1)); }

    T *take(uint i)
    {
        if (qsizetype(i) >= v.size())
            return nullptr;
        T *p = v.takeAt(qsizetype(i));         // never deletes
        cursorAfterRemoval(qsizetype(i));
        return p;
    }
    T *take() { return valid() ? take(uint(cur)) : nullptr; }

    void clear() { if (del) qDeleteAll(v); v.clear(); cur = -1; }

    // Phase B: the owner frees its items explicitly.
    void deleteContents() { qDeleteAll(v); v.clear(); cur = -1; }
    // No sort(): Qt 2's QGList::sort uses compareItems, which for QList<T> is
    // 'item1 != item2' and never returns negative (qglist.cpp:125-128), so it
    // produces an arbitrary permutation. Sorting by pointer address would be a
    // different arbitrary one. No SIGEL caller exists; add a real comparator if
    // one ever does.

    const QList<T *> &constList() const { return v; }   // for Q2ListIterator

private:
    bool valid() const { return cur >= 0 && cur < v.size(); }

    // qglist.cpp:436-473.  The cursor is already at the removal site (locate()
    // ran first), so it lands on whatever slid in, or steps back if that was
    // the last element.
    void cursorAfterRemoval(qsizetype removed)
    {
        if (removed < v.size())      cur = removed;
        else if (!v.isEmpty())       cur = v.size() - 1;
        else                         cur = -1;
    }

    QList<T *> v;
    qsizetype cur = -1;
    bool del = false;
};

// ---------------------------------------------------------------------------
// Q2ListIterator<T>  <- Qt 2 QListIterator<T>.  Independent of the list's own
// cursor, which is the point of it.  Qt 2 repairs registered iterators across
// mutation (qglist.cpp:463-470); this one does not, so do not mutate the list
// while iterating.  No core site does.
// ---------------------------------------------------------------------------
template <class T>
class Q2ListIterator
{
public:
    Q2ListIterator(const Q2PtrList<T> &l) : v(&l.constList()), i(0) {}

    uint count() const { return uint(v->size()); }
    bool isEmpty() const { return v->isEmpty(); }
    bool atFirst() const { return v->isEmpty() || i == 0; }   // qglist.h:241-244
    bool atLast() const { return v->isEmpty() || i == v->size() - 1; }   // qglist.h:246-249

    T *current() const { return (i >= 0 && i < v->size()) ? v->at(i) : nullptr; }
    T *toFirst() { i = 0; return current(); }
    T *toLast() { i = v->size() - 1; return current(); }

    // Once off either end the iterator stays dead, as in Qt 2: every mover
    // there begins 'if (!curNode) return 0' (qglist.cpp:1166-1211).
    T *operator++() { if (!current()) return nullptr; ++i; return current(); }
    T *operator--() { if (!current()) return nullptr; --i; return current(); }
    T *operator()() { T *p = current(); if (p) ++i; return p; }

private:
    const QList<T *> *v;
    qsizetype i;
};

// ---------------------------------------------------------------------------
// Q2Queue<T>  <- Qt 2 QQueue<T> (qqueue.h:46-67): private QGList holding
// *pointers* with autoDelete.  Qt 6's QQueue holds values -- the same silent
// rename trap as QVector/QList.
// ---------------------------------------------------------------------------
template <class T>
class Q2Queue
{
public:
    uint count() const { return l.count(); }
    bool isEmpty() const { return l.isEmpty(); }
    void setAutoDelete(bool enable) { l.setAutoDelete(enable); }
    bool autoDelete() const { return l.autoDelete(); }

    void enqueue(const T *d) { l.append(d); }

    // Unlinks even when the head is null. Qt 2's QQueue::dequeue is
    // QGList::takeFirst (qqueue.h:59), which removes the first NODE regardless
    // of whether the item it holds is null. Testing the item instead would make
    // 'while (!isEmpty()) dequeue();' spin forever on a queued null.
    T *dequeue()
    {
        if (l.isEmpty())
            return nullptr;
        return l.take(0u);
    }

    bool remove() { return l.removeFirst(); }   // qqueue.h:60
    T *head() const { return l.getFirst(); }
    T *current() const { return l.getFirst(); }
    operator T *() const { return l.getFirst(); }
    void clear() { l.clear(); }

private:
    Q2PtrList<T> l;
};

// ---------------------------------------------------------------------------
// Q2ValueList<T>  <- Qt 2 QValueList<T>: a VALUE list, but a doubly-LINKED one
// (qvaluelist.h:51-62), so an iterator stays valid when the list is modified
// elsewhere. Qt 6's QList is contiguous and reallocates, which invalidates
// every iterator into it.
//
// That difference is not academic here: SIG_GPManager.cpp:106-217 walks
// taskCanDoList with an iterator while APPENDING to the same list inside the
// loop, then passes the iterator to remove(). Backed by QList that is a
// use-after-free. std::list reproduces Qt 2's node semantics exactly.
// ---------------------------------------------------------------------------
template <class T>
class Q2ValueList
{
    std::list<T> l;

public:
    typedef typename std::list<T>::iterator Iterator;
    typedef typename std::list<T>::const_iterator ConstIterator;
    typedef T ValueType;

    Iterator begin() { return l.begin(); }
    Iterator end() { return l.end(); }
    ConstIterator begin() const { return l.begin(); }
    ConstIterator end() const { return l.end(); }

    uint count() const { return uint(l.size()); }
    uint size() const { return uint(l.size()); }
    bool isEmpty() const { return l.empty(); }
    void clear() { l.clear(); }

    void append(const T &d) { l.push_back(d); }
    void prepend(const T &d) { l.push_front(d); }
    Q2ValueList<T> &operator<<(const T &d) { l.push_back(d); return *this; }

    T &first() { return l.front(); }
    T &last() { return l.back(); }

    // Qt 2 returns the following iterator (qvaluelist.h:375, body 225-234)
    Iterator remove(Iterator it) { return l.erase(it); }
    void remove(const T &d) { l.remove(d); }

    uint contains(const T &d) const   // Qt 2 returns a COUNT, not a bool
    {
        uint n = 0;
        for (ConstIterator i = l.begin(); i != l.end(); ++i)
            if (*i == d)
                ++n;
        return n;
    }

    // SIG_AllIndividualsView.cpp:119 and SIG_MainWindow.cpp:64 hand one of
    // these to QSplitter::setSizes, which wants a QList.
    operator QList<T>() const { return QList<T>(l.begin(), l.end()); }
};

// ---------------------------------------------------------------------------
// Q2CString  <- Qt 2 QCString.  Qt 6's QByteArray is equivalent except that it
// has no implicit conversion to const char*, which every SIGEL use relies on:
//   Q2CString s = str.toUtf8();  char const *c = s;
// ---------------------------------------------------------------------------
class Q2CString : public QByteArray
{
public:
    Q2CString() {}
    Q2CString(const QByteArray &b) : QByteArray(b) {}
    Q2CString(const char *s) : QByteArray(s) {}

    // Qt 2's QCString is a QArray<char> whose buffer INCLUDES the terminating
    // NUL (qcstring.h:156,175), so size() and count() are length()+1 for a
    // non-null string. These hide QByteArray's versions, which is NOT fully
    // safe: SIG_GPFitnessTrainer.cpp:319 streams one into a QTextStream and
    // overload resolution binds the QByteArray& overload, so that path sees
    // QByteArray::size(). Benign there, but the hiding IS visible via a base
    // reference.
    uint size() const { return isNull() ? 0u : uint(QByteArray::size()) + 1u; }
    uint count() const { return size(); }
    bool resize(uint n) { QByteArray::resize(n ? qsizetype(n) - 1 : 0); return true; }

    // Qt 2 returns the number of occurrences, not a bool.
    uint contains(char c) const { return uint(QByteArray::count(c)); }

    // NOTE: operator[] is QByteArray's, bounds-checked against
    // QByteArray::size() -- one LESS than the size() above. Indexing the
    // terminating NUL, which Qt 2 allowed, therefore asserts. No SIGEL site
    // indexes a QCString; if one appears, give it its own operator[] rather
    // than relying on this.

    // qcstring.h:310 returns data(), which is null for a null string.
    operator const char *() const { return isNull() ? nullptr : constData(); }
};

#endif // Q2COMPAT_H
