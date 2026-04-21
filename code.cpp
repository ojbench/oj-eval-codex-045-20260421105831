#include <cstdio>
#include <iostream>

using namespace std;

// Custom open-addressing hash map from int -> unsigned char (0/1)
struct HashMap {
    static const int POW = 21; // 2^21 = 2,097,152 > 2 * 5e5
    static const int SZ = 1 << POW;
    static const int MASK = SZ - 1;
    int keys[SZ];
    unsigned char used[SZ];
    unsigned char vals[SZ];

    inline unsigned int h(unsigned int x) const {
        // Knuth multiplicative hash for 32-bit
        return (x * 2654435761u) & MASK;
    }

    // find position of key or first empty slot
    int find_pos(int k) {
        unsigned int i = h((unsigned int)k);
        while (used[i] && keys[i] != k) {
            i = (i + 1) & MASK;
        }
        return (int)i;
    }

    inline unsigned char get(int k) {
        unsigned int i = h((unsigned int)k);
        while (used[i]) {
            if (keys[i] == k) return vals[i];
            i = (i + 1) & MASK;
        }
        return 0; // default not present
    }

    inline void set(int k, unsigned char v) {
        int i = find_pos(k);
        if (!used[i]) {
            used[i] = 1;
            keys[i] = k;
        }
        vals[i] = v;
    }
} present;

// Min-heap (for requests above current floor)
struct MinHeap {
    int sz;
    int a[600000 + 5];
    MinHeap(): sz(0) {}
    inline bool empty() const { return sz == 0; }
    inline int top() const { return a[1]; }
    inline void push(int x) {
        int i = ++sz;
        while (i > 1) {
            int p = i >> 1;
            if (a[p] <= x) break;
            a[i] = a[p];
            i = p;
        }
        a[i] = x;
    }
    inline void pop() {
        int x = a[sz--];
        int i = 1;
        while ((i << 1) <= sz) {
            int l = i << 1, r = l + 1, m = l;
            if (r <= sz && a[r] < a[l]) m = r;
            if (a[m] >= x) break;
            a[i] = a[m];
            i = m;
        }
        if (sz >= 0) a[i] = x;
    }
} upHeap;

// Max-heap (for requests below current floor)
struct MaxHeap {
    int sz;
    int a[600000 + 5];
    MaxHeap(): sz(0) {}
    inline bool empty() const { return sz == 0; }
    inline int top() const { return a[1]; }
    inline void push(int x) {
        int i = ++sz;
        while (i > 1) {
            int p = i >> 1;
            if (a[p] >= x) break;
            a[i] = a[p];
            i = p;
        }
        a[i] = x;
    }
    inline void pop() {
        int x = a[sz--];
        int i = 1;
        while ((i << 1) <= sz) {
            int l = i << 1, r = l + 1, m = l;
            if (r <= sz && a[r] > a[l]) m = r;
            if (a[m] <= x) break;
            a[i] = a[m];
            i = m;
        }
        if (sz >= 0) a[i] = x;
    }
} downHeap;

// Clean invalid/crossed items and peek next valid target above current
inline int peekValidUp(int cur) {
    while (!upHeap.empty()) {
        int t = upHeap.top();
        if (t <= cur || present.get(t) == 0) { upHeap.pop(); continue; }
        return t;
    }
    return -1;
}

// Clean invalid/crossed items and peek next valid target below current
inline int peekValidDown(int cur) {
    while (!downHeap.empty()) {
        int t = downHeap.top();
        if (t >= cur || present.get(t) == 0) { downHeap.pop(); continue; }
        return t;
    }
    return -1;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;
    long long cur = 0; // current floor
    bool dirUp = true; // true: up, false: down

    char op[16];
    for (int i = 0; i < n; ++i) {
        cin >> op;
        if (op[0] == 'a') { // action or add
            if (op[1] == 'c') { // action
                if (dirUp) {
                    int u = peekValidUp((int)cur);
                    if (u != -1) {
                        upHeap.pop();
                        cur = u;
                        present.set(u, 0);
                    } else {
                        int d = peekValidDown((int)cur);
                        if (d != -1) {
                            dirUp = false;
                            downHeap.pop();
                            cur = d;
                            present.set(d, 0);
                        } else {
                            // no pending requests, stay and keep direction
                        }
                    }
                } else { // dir down
                    int d = peekValidDown((int)cur);
                    if (d != -1) {
                        downHeap.pop();
                        cur = d;
                        present.set(d, 0);
                    } else {
                        int u = peekValidUp((int)cur);
                        if (u != -1) {
                            dirUp = true;
                            upHeap.pop();
                            cur = u;
                            present.set(u, 0);
                        } else {
                            // no pending requests
                        }
                    }
                }
            } else { // add x
                int x; cin >> x;
                // guarantee x != cur
                present.set(x, 1);
                if (x > cur) upHeap.push(x);
                else /* x < cur */ downHeap.push(x);
            }
        } else if (op[0] == 'c') { // cancel x
            int x; cin >> x;
            present.set(x, 0);
        } else { // locate
            // op == locate
            cout << cur << '\n';
        }
    }
    return 0;
}

