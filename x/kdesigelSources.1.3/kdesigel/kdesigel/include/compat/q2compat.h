#ifndef Q2COMPAT_H
#define Q2COMPAT_H

// Qt 2.3 -> Qt 6 compatibility shim.  PORTING.md D1(a), step A0.
//
// Reproduces Qt 2.3 container semantics on top of Qt 6 so that steps A1-A9 are
// pure renames and every ownership decision defers to Phase B, which deletes
// this file.
//
//   QArray      -> Q2Array          QVector     -> Q2PtrVector
//   QDict       -> Q2Dict           QList       -> Q2PtrList
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
// DELIBERATE DIVERGENCES FROM Qt 2 -- all four are decisions, see PORTING.md section 9:
//
//  1. sort() is numeric, not memcmp byte order.  Qt 2's QGArray is a type-erased
//     byte buffer whose sort cannot know it holds int (qgarray.cpp:635-640; the
//     Qt source itself notes "Qt 3.0: Add a virtual compareItems()").  Q2Array<T>
//     is typed, so std::sort with operator< removes the cause.  Affects
//     SIG_GPManager.cpp:304,311 and SIG_AllIndividualsView.cpp:240, all of
//     which need ascending numeric order and silently break above 256.
//  2. Out-of-range indexing warns and clamps to index 0, as Qt 2's QGArray::at
//     did (qgarray.h:108-117). Qt 2's QGVector::at only warned and then read out
//     of range anyway (qgvector.h:85-92); clamping is used for both. This is
//     done in the shim rather than left to Q_ASSERT, which compiles to nothing
//     under QT_NO_DEBUG and would make a release build corrupt memory silently
//     where 2003 merely returned a wrong value. Call sites are still being
//     fixed; the clamp is a floor, not a licence.
//  3. resize() value-initialises new elements where Qt 2 left raw memory.
//  Two Qt 2 quirks are deliberately NOT reproduced, because no SIGEL code can
//  reach them and both are defects rather than behaviour:
//    - Copying a Qt 2 QDict REVERSES the order of same-key entries, so find()
//      returns the oldest afterwards. Unreachable here: every dict is keyed by a
//      model name, insert() is unguarded, and no shipped .rrb file contains a
//      duplicate name.
//    - Qt 2's QValueList::contains returns an occurrence count; Qt 6's returns
//      bool. Q2ValueList has exactly one declaration in SIGEL and contains() is
//      never called on it.
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
#include <QMultiHash>
#include <QPair>
#include <QString>
#include <algorithm>

// ---------------------------------------------------------------------------
// Qt 6 randomises the QHash seed once per process, so Q2Dict iteration order
// differs between runs. Qt 2's QGDict walked its buckets in a fixed order for a
// given insertion sequence, and SIGEL depends on that: SIG_Robot's copy
// constructor is a serialise/deserialise round-trip that iterates six dicts
// (SIG_Robot.cpp:295-335), and reading back registers joints with their links
// in encounter order. Without a fixed seed, adjacentJoints ordering -- and with
// it the MDH traversal and DynaMechs link numbering -- varies run to run, so a
// fixed RANDOMSEED no longer reproduces a run.
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

// ---------------------------------------------------------------------------
// Q2Array<T>  <- Qt 2 QArray<T> (QMemArray): value array.
// qarray.h: size() == count() == element count.
// ---------------------------------------------------------------------------
template <class T>
class Q2Array
{
public:
    typedef T *Iterator;
    typedef const T *ConstIterator;
    typedef T ValueType;

    Q2Array() {}
    explicit Q2Array(int size) { if (size > 0) m.resize(size); }  // Qt 2 clamps <0 to 0

    uint size() const { return uint(m.size()); }
    uint count() const { return uint(m.size()); }
    bool isEmpty() const { return m.isEmpty(); }
    bool isNull() const { return m.isEmpty(); }

    bool resize(uint size) { m.resize(qsizetype(size)); return true; }      // divergence 3
    bool truncate(uint pos) { m.resize(qsizetype(pos)); return true; }

    bool fill(const T &d, int size = -1)
    {
        if (size >= 0)
            m.resize(qsizetype(size));
        m.fill(d);
        return true;
    }

    void clear() { m.clear(); }
    void detach() { m.detach(); }
    Q2Array<T> copy() const { Q2Array<T> t; t.m = m; t.m.detach(); return t; }

    // Qt 2 hands out a non-const T& from a const array (qarray.h:108-117).
    // Preserved so A-step renames stay pure.  See divergence 4 on sharing.
    // qgarray.h:108-117 -- Qt 2 warns and CLAMPS the index to 0 rather than
    // failing. Reproduced here rather than left to Q_ASSERT, which compiles to
    // nothing under QT_NO_DEBUG and would give silent corruption in a release
    // build. An empty array has no element to clamp to, so it yields a dummy.
    T &at(uint i) const
    {
        Q2Array<T> *self = const_cast<Q2Array<T> *>(this);
        if (qsizetype(i) < self->m.size())
            return self->m[qsizetype(i)];
        qWarning("Q2Array::at: index %u out of range (size %lld)",
                 i, static_cast<long long>(self->m.size()));
        if (self->m.isEmpty()) {
            static T dummy = T();
            return dummy;
        }
        return self->m[0];
    }
    T &operator[](int i) const { return at(uint(i)); }
    T *data() const { return const_cast<Q2Array<T> *>(this)->m.data(); }
    operator const T *() const { return m.isEmpty() ? nullptr : m.constData(); }

    int find(const T &d, uint i = 0) const { return int(m.indexOf(d, qsizetype(i))); }
    int contains(const T &d) const { return int(m.count(d)); }
    void sort() { std::sort(m.begin(), m.end()); }                          // divergence 1

    bool operator==(const Q2Array<T> &a) const { return m == a.m; }
    bool operator!=(const Q2Array<T> &a) const { return m != a.m; }

private:
    QList<T> m;
};

// ---------------------------------------------------------------------------
// Q2Dict<T>  <- Qt 2 QDict<T>: QString-keyed dictionary of *pointers*.
// qdict.h: count() is the item count, size() is the hash table size.
// Duplicate keys are allowed and stack newest-first (qgdict.cpp:379-386),
// hence QMultiHash.
// ---------------------------------------------------------------------------
template <class T>
class Q2Dict
{
public:
    typedef QMultiHash<QString, T *> HashType;

    explicit Q2Dict(int size = 17) : buckets(uint(size)) {}

    // qcollection.h:64 -- QCollection(const QCollection&) { del_item = FALSE; }
    // A Qt 2 copy is ALWAYS non-owning.  Omitting this is a double-free.
    Q2Dict(const Q2Dict &o) : h(o.h), buckets(o.buckets), del(false) {}

    // qgdict.cpp:280-302 -- clear() the destination first (honouring ITS
    // autoDelete), copy, and leave the destination's flag untouched.
    Q2Dict &operator=(const Q2Dict &o)
    {
        if (this != &o) { clear(); h = o.h; }   // Qt 2 leaves vlen untouched
        return *this;
    }

    ~Q2Dict() { if (del) qDeleteAll(h); }

    uint count() const { return uint(h.size()); }
    uint size() const { return buckets; }   // hash table size, not item count
    bool isEmpty() const { return h.isEmpty(); }

    void setAutoDelete(bool enable) { del = enable; }
    bool autoDelete() const { return del; }

    void insert(const QString &k, const T *d) { h.insert(k, const_cast<T *>(d)); }

    void replace(const QString &k, const T *d) { remove(k); h.insert(k, const_cast<T *>(d)); }

    T *find(const QString &k) const
    {
        typename HashType::const_iterator it = h.constFind(k);
        return it == h.constEnd() ? nullptr : it.value();
    }

    // Qt 2 removes ONE item (the most recent for that key), not all of them.
    bool remove(const QString &k)
    {
        typename HashType::iterator it = h.find(k);
        if (it == h.end())
            return false;
        if (del)
            delete it.value();
        h.erase(it);
        return true;
    }

    T *take(const QString &k)
    {
        typename HashType::iterator it = h.find(k);
        if (it == h.end())
            return nullptr;
        T *v = it.value();
        h.erase(it);
        return v;
    }

    void clear() { if (del) qDeleteAll(h); h.clear(); }
    void resize(uint n) { buckets = n; h.reserve(qsizetype(n)); }

    const HashType &constHash() const { return h; }   // for Q2DictIterator

private:
    HashType h;
    uint buckets;
    bool del = false;
};

// ---------------------------------------------------------------------------
// Q2DictIterator<T>  <- Qt 2 QDictIterator<T>.
//
// Qt 2 registers every iterator with its dict and repairs them on mutation
// (qgdict.cpp:583-590), so a Qt 2 iterator survives erase and returns 0 rather
// than dereferencing freed memory.  Qt 6's QMultiHash iterators are simply
// invalidated by any erase or rehash -- use-after-free, not a graceful null.
//
// This snapshots the dict at construction instead.  No core site mutates a
// dict while iterating it, so behaviour is identical for every real use, and
// it is crash-safe rather than undefined if one is ever added.  Being a value
// makes it trivially copyable, which SIG_Link::getPointIter needs -- it
// returns an iterator BY VALUE.
// ---------------------------------------------------------------------------
template <class T>
class Q2DictIterator
{
public:
    Q2DictIterator(const Q2Dict<T> &d) : dict(&d), i(0)
    {
        const typename Q2Dict<T>::HashType &h = d.constHash();
        snap.reserve(h.size());
        for (typename Q2Dict<T>::HashType::const_iterator it = h.constBegin();
             it != h.constEnd(); ++it)
            snap.append(qMakePair(it.key(), it.value()));
    }

    // Qt 2 answers these from the live dict, not from the iteration position
    uint count() const { return dict->count(); }
    bool isEmpty() const { return dict->count() == 0; }

    T *current() const { return valid() ? snap.at(i).second : nullptr; }
    QString currentKey() const { return valid() ? snap.at(i).first : QString(); }

    T *toFirst() { i = 0; return current(); }

    T *operator++() { if (i < snap.size()) ++i; return current(); }          // advance, return new
    T *operator()() { T *v = current(); if (i < snap.size()) ++i; return v; } // return old, advance

private:
    bool valid() const { return i >= 0 && i < snap.size(); }
    const Q2Dict<T> *dict;
    QList<QPair<QString, T *> > snap;
    qsizetype i;
};

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
    // sync, and this class dies in Phase B.  Two hot spots if that ever matters:
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
    // qglist.cpp:504-509 -- a null argument does NOT search; it removes current.
    bool remove(const T *d)
    {
        if (!d)
            return remove();
        const qsizetype i = v.indexOf(const_cast<T *>(d));
        return i < 0 ? false : remove(uint(i));
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
    bool atFirst() const { return v->isEmpty() || i == 0; }   // qglist.h:246-249
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

    // Unlinks even when the head is null, as QGList::dequeue does
    // (qglist.cpp:623-630). Returning early on a null head would make
    // 'while (!isEmpty()) dequeue();' spin forever.
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
// Q2ValueList<T>  <- Qt 2 QValueList<T>: a VALUE list, so Qt 6's QList already
// matches.  Only remove(iterator) differs -- Qt 2 returns the following
// iterator, Qt 6 spells that erase().
// ---------------------------------------------------------------------------
template <class T>
class Q2ValueList : public QList<T>
{
public:
    typedef typename QList<T>::iterator Iterator;
    typedef typename QList<T>::const_iterator ConstIterator;

    Iterator remove(Iterator it) { return QList<T>::erase(it); }
    void remove(const T &d) { QList<T>::removeAll(d); }
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
    // non-null string. These hide QByteArray's versions deliberately; that is
    // only safe because SIGEL never handles one through a QByteArray reference.
    uint size() const { return isNull() ? 0u : uint(QByteArray::size()) + 1u; }
    uint count() const { return size(); }
    bool resize(uint n) { QByteArray::resize(n ? qsizetype(n) - 1 : 0); return true; }

    // Qt 2 returns the number of occurrences, not a bool.
    uint contains(char c) const { return uint(QByteArray::count(c)); }

    // qcstring.h:310 returns data(), which is null for a null string.
    operator const char *() const { return isNull() ? nullptr : constData(); }
};

#endif // Q2COMPAT_H
