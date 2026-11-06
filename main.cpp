// Hash Verse Visualizer - console edition
//
// Build:  g++ -std=c++17 -Wall -Wextra -O2 -o hash_verse main.cpp
// Run:    ./hash_verse
//
// Five demos, same as the web version:
//   1. Memory Grid        - hash table with linear/quadratic probing and chaining
//   2. Password Avalanche - a one-character change flips about half of the hash bits
//   3. Plagiarism Scanner - rolling-hash (Rabin-Karp) text matching
//   4. LRU Cache          - least-recently-used eviction
//   5. Break the Hash     - find a word that lands in a chosen box

#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

// ---------- terminal colours (ANSI escape codes) ----------
namespace clr {
const string RESET    = "\033[0m";
const string BOLD     = "\033[1m";
const string GRAY     = "\033[90m";
const string RED      = "\033[31m";
const string GREEN    = "\033[32m";
const string YELLOW   = "\033[33m";
const string CYAN     = "\033[36m";
const string BLACK    = "\033[30m";
const string WHITE    = "\033[97m";
const string BG_RED    = "\033[41m";
const string BG_GREEN  = "\033[42m";
const string BG_YELLOW = "\033[43m";
const string BG_BLUE   = "\033[44m";
}  // namespace clr

// ---------- small helpers ----------
static string trim(const string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

static string readLine(const string& prompt) {
    cout << prompt;
    string line;
    if (!getline(cin, line)) exit(0);  // end of input (Ctrl+D)
    return trim(line);
}

static int readInt(const string& prompt) {
    string s = readLine(prompt);
    try {
        return stoi(s);
    } catch (...) {
        return -1;
    }
}

static void header(const string& title) {
    cout << "\n" << clr::BOLD << clr::CYAN << "=== " << title << " ===" << clr::RESET << "\n";
}
static void ok(const string& s)   { cout << clr::GREEN  << s << clr::RESET << "\n"; }
static void warn(const string& s) { cout << clr::YELLOW << s << clr::RESET << "\n"; }
static void err(const string& s)  { cout << clr::RED    << s << clr::RESET << "\n"; }
static void info(const string& s) { cout << clr::CYAN   << s << clr::RESET << "\n"; }

static string fmt2(double v) {
    ostringstream os;
    os << fixed << setprecision(2) << v;
    return os.str();
}

// Sum of character codes: "cat" = 99 + 97 + 116 = 312
static unsigned long long asciiSum(const string& s) {
    unsigned long long t = 0;
    for (unsigned char c : s) t += c;
    return t;
}

static bool isPrime(size_t n) {
    if (n < 2) return false;
    for (size_t i = 2; i * i <= n; i++)
        if (n % i == 0) return false;
    return true;
}

static size_t nextPrime(size_t n) {
    while (!isPrime(n)) n++;
    return n;
}

// ======================================================================
// 1. Memory Grid - hash table
// ======================================================================
enum class Method { Linear = 1, Quadratic = 2, Chaining = 3 };

static string methodName(Method m) {
    switch (m) {
        case Method::Linear:    return "Linear probing";
        case Method::Quadratic: return "Quadratic probing";
        default:                return "Separate chaining";
    }
}

class HashTable {
public:
    explicit HashTable(Method m) : method_(m) { reset(); }

    void reset() {
        cap_ = 7;
        slots_.assign(cap_, "");
        used_.assign(cap_, false);
        chains_.assign(cap_, vector<string>());
        count_ = 0;
        lastOk_ = lastHot_ = -1;
    }

    void setMethod(Method m) {
        method_ = m;
        reset();
    }

    Method method() const { return method_; }

    void insert(const string& w) {
        lastOk_ = lastHot_ = -1;
        if (contains(w)) {
            warn("\"" + w + "\" already exists in the table.");
            return;
        }

        if (double(count_ + 1) / cap_ >= 0.7) {
            warn("Load factor would reach " + fmt2(double(count_ + 1) / cap_) +
                 " (>= 0.70). Growing table...");
            rehash(nullptr);
        }

        size_t h = bucket(w);
        cout << "ASCII sum of \"" << w << "\" = " << asciiSum(w) << "\n";
        cout << "Index = " << asciiSum(w) << " mod " << cap_ << " = " << h << "\n";

        if (method_ == Method::Chaining) {
            if (!chains_[h].empty())
                err("Collision at box " + to_string(h) + ". Added to the chain.");
            else
                ok("Placed in box " + to_string(h) + ".");
            chains_[h].push_back(w);
            ++count_;
            lastOk_ = static_cast<long long>(h);
            return;
        }

        for (size_t p = 0; p < cap_; p++) {
            size_t i = probe(h, p, cap_);
            if (!used_[i]) {
                used_[i] = true;
                slots_[i] = w;
                ++count_;
                lastOk_ = static_cast<long long>(i);
                ok("Box " + to_string(i) + " is empty. Placed \"" + w + "\" in box " +
                   to_string(i) + ".");
                return;
            }
            lastHot_ = static_cast<long long>(i);
            err("Collision at box " + to_string(i) + " (holds \"" + slots_[i] + "\"). " +
                (method_ == Method::Quadratic ? "Quadratic" : "Linear") +
                " probe, next try...");
        }

        // Every probe position was taken: grow the table and place the word.
        warn("No free spot on the probe path. Growing the table...");
        rehash(&w);
        lastHot_ = -1;
        lastOk_ = locate(w);
    }

    void search(const string& w) {
        lastOk_ = lastHot_ = -1;
        cout << "Index = " << asciiSum(w) << " mod " << cap_ << " = " << bucket(w) << "\n";
        size_t steps = 0;
        long long at = locate(w, &steps);
        if (at >= 0) {
            lastOk_ = at;
            if (method_ == Method::Chaining)
                ok("Found in the chain at box " + to_string(at) + ".");
            else
                ok("Found at box " + to_string(at) + " after " + to_string(steps) + " step(s).");
        } else {
            err("Not found.");
        }
    }

    void draw() const {
        cout << "\n";
        for (size_t i = 0; i < cap_; i++) {
            string content;
            bool full;
            if (method_ == Method::Chaining) {
                full = !chains_[i].empty();
                for (size_t k = 0; k < chains_[i].size(); k++) {
                    if (k) content += " -> ";
                    content += chains_[i][k];
                }
            } else {
                full = used_[i];
                if (full) content = slots_[i];
            }

            string style;
            if (static_cast<long long>(i) == lastHot_)     style = clr::BG_RED + clr::WHITE;
            else if (static_cast<long long>(i) == lastOk_) style = clr::BG_GREEN + clr::BLACK;
            else if (full)                                 style = clr::BG_BLUE + clr::WHITE;
            else                                           style = clr::GRAY;

            string text = "[" + to_string(i) + "]  " + (content.empty() ? "[empty]" : content);
            if (text.size() < 36) text.append(36 - text.size(), ' ');
            cout << "  " << style << " " << text << " " << clr::RESET << "\n";
        }
        cout << clr::GRAY << "Table size: " << cap_ << "   Items: " << count_
             << "   Load factor: " << fmt2(double(count_) / cap_) << clr::RESET << "\n";
    }

private:
    Method method_;
    size_t cap_ = 7;
    vector<string> slots_;
    vector<bool> used_;
    vector<vector<string>> chains_;
    size_t count_ = 0;
    long long lastOk_ = -1;
    long long lastHot_ = -1;

    size_t bucket(const string& w) const { return asciiSum(w) % cap_; }

    size_t probe(size_t h, size_t p, size_t n) const {
        return method_ == Method::Quadratic ? (h + p * p) % n : (h + p) % n;
    }

    // Returns the slot index holding w, or -1. Optionally reports the number of steps.
    long long locate(const string& w, size_t* steps = nullptr) const {
        size_t h = bucket(w);
        if (method_ == Method::Chaining) {
            if (steps) *steps = 1;
            const auto& c = chains_[h];
            return std::find(c.begin(), c.end(), w) != c.end() ? static_cast<long long>(h) : -1;
        }
        for (size_t p = 0; p < cap_; p++) {
            size_t i = probe(h, p, cap_);
            if (!used_[i]) break;
            if (slots_[i] == w) {
                if (steps) *steps = p + 1;
                return static_cast<long long>(i);
            }
        }
        return -1;
    }

    bool contains(const string& w) const { return locate(w) >= 0; }

    // Tries to place all items into a table of size n. Fails if probing runs out of room.
    bool build(size_t n, const vector<string>& items, vector<string>& s,
               vector<bool>& u, vector<vector<string>>& c) const {
        s.assign(n, "");
        u.assign(n, false);
        c.assign(n, vector<string>());
        for (const auto& w : items) {
            size_t h = asciiSum(w) % n;
            if (method_ == Method::Chaining) {
                c[h].push_back(w);
                continue;
            }
            bool placed = false;
            for (size_t p = 0; p < n; p++) {
                size_t i = probe(h, p, n);
                if (!u[i]) {
                    u[i] = true;
                    s[i] = w;
                    placed = true;
                    break;
                }
            }
            if (!placed) return false;
        }
        return true;
    }

    // Grows the table to the next prime >= 2 * size and re-inserts every word.
    void rehash(const string* extra) {
        vector<string> items;
        for (size_t i = 0; i < cap_; i++)
            if (used_[i]) items.push_back(slots_[i]);
        for (const auto& c : chains_) items.insert(items.end(), c.begin(), c.end());
        if (extra) items.push_back(*extra);

        size_t nc = nextPrime(cap_ * 2);
        vector<string> s;
        vector<bool> u;
        vector<vector<string>> c;
        while (!build(nc, items, s, u, c)) nc = nextPrime(nc + 1);

        cap_ = nc;
        slots_ = move(s);
        used_ = move(u);
        chains_ = move(c);
        count_ = items.size();
        ok("Rehash done. New table size = " + to_string(cap_) + " (next prime of double).");
    }
};

static void memoryGrid() {
    header("1. Memory Grid");
    info("Words go into numbered boxes. Green = just placed, red = collision, blue = used.");
    HashTable table(Method::Linear);
    while (true) {
        cout << "\nMethod: " << clr::CYAN << methodName(table.method()) << clr::RESET << "\n";
        table.draw();
        cout << " 1) Insert   2) Search   3) Change method   4) Reset   0) Back\n";
        int c = readInt("> ");
        if (c == 0) return;
        if (c == 1 || c == 2) {
            string w = readLine("Word: ");
            if (w.empty()) continue;
            if (c == 1) table.insert(w);
            else        table.search(w);
        } else if (c == 3) {
            cout << " 1) Linear  2) Quadratic  3) Chaining\n";
            int m = readInt("Method: ");
            if (m >= 1 && m <= 3) {
                table.setMethod(static_cast<Method>(m));
                warn("Method changed. The table was cleared.");
            } else {
                err("Invalid method.");
            }
        } else if (c == 4) {
            table.reset();
            ok("Table reset.");
        } else {
            err("Invalid choice.");
        }
    }
}

// ======================================================================
// 2. Password Avalanche
// ======================================================================
// FNV-1a with a final bit-mixing step, so every input change spreads across all 64 bits.
static uint64_t hashPassword(const string& s) {
    uint64_t h = 14695981039346656037ULL;  // FNV offset basis
    for (unsigned char c : s) {
        h ^= c;
        h *= 1099511628211ULL;  // FNV prime
    }
    h ^= h >> 33;
    h *= 0xff51afd7ed558ccdULL;
    h ^= h >> 33;
    h *= 0xc4ceb9fe1a85ec53ULL;
    h ^= h >> 33;
    return h;
}

static string toBinary(uint64_t v) {
    string s(64, '0');
    for (int i = 0; i < 64; i++) s[63 - i] = ((v >> i) & 1) ? '1' : '0';
    return s;
}

static string hex16(uint64_t v) {
    ostringstream os;
    os << "0x" << hex << setw(16) << setfill('0') << v;
    return os.str();
}

static void passwordAvalanche() {
    header("2. Password Avalanche");
    info("Change one character and about half of the 64 bits change.");
    string p1 = readLine("Password 1 (Enter = Hello123): ");
    if (p1.empty()) p1 = "Hello123";
    string p2 = readLine("Password 2 (Enter = Hello124): ");
    if (p2.empty()) p2 = "Hello124";

    uint64_t a = hashPassword(p1), b = hashPassword(p2);
    string A = toBinary(a), B = toBinary(b);
    int changed = 0;
    for (int i = 0; i < 64; i++)
        if (A[i] != B[i]) changed++;

    cout << "\nHash 1: " << hex16(a) << "\n" << A << "\n";
    cout << "Hash 2: " << hex16(b) << "\n";
    for (int i = 0; i < 64; i++) {
        if (A[i] != B[i]) cout << clr::BG_RED << clr::WHITE << B[i] << clr::RESET;
        else              cout << B[i];
    }
    cout << "\n\n";

    ostringstream pct;
    pct << fixed << setprecision(0) << changed * 100.0 / 64;
    ok("Changed bits: " + to_string(changed) + " of 64 (" + pct.str() + "%)");
    if (p1 == p2) info("Same input gives the same hash.");
    else cout << "Red bits differ. Demo only, not real security (use bcrypt or argon2 for passwords).\n";
}

// ======================================================================
// 3. Plagiarism Scanner (Rabin-Karp rolling hash)
// ======================================================================
static void plagiarismScanner() {
    header("3. Plagiarism Scanner");
    info("Finds copied pieces by comparing rolling hashes of small text windows.");
    string original = readLine("Original text: ");
    string suspect = readLine("Suspect text: ");
    int k = readInt("Window size 2-12 (Enter = 5): ");
    if (k < 2 || k > 12) k = 5;

    const unsigned long long B = 257, M = 1000000007ULL;

    // Hash of every window of length k, updated in O(1) per step.
    auto windows = [&](const string& s) {
        vector<unsigned long long> r;
        if (static_cast<int>(s.size()) < k) return r;
        unsigned long long h = 0, p = 1;  // p = B^(k-1)
        for (int i = 0; i < k; i++) {
            h = (h * B + static_cast<unsigned char>(s[i]) + 1) % M;
            if (i) p = p * B % M;
        }
        r.push_back(h);
        for (size_t i = k; i < s.size(); i++) {
            unsigned long long drop = (static_cast<unsigned char>(s[i - k]) + 1) * p % M;
            h = (h + M - drop) % M;
            h = (h * B + static_cast<unsigned char>(s[i]) + 1) % M;
            r.push_back(h);
        }
        return r;
    };

    vector<unsigned long long> wa = windows(original), wb = windows(suspect);
    unordered_map<unsigned long long, vector<size_t>> at;
    for (size_t j = 0; j < wb.size(); j++) at[wb[j]].push_back(j);

    vector<bool> mark(original.size(), false);
    for (size_t i = 0; i < wa.size(); i++) {
        auto it = at.find(wa[i]);
        if (it == at.end()) continue;
        for (size_t j : it->second) {
            // Same hash does not always mean same text, so compare the letters too.
            if (original.compare(i, k, suspect, j, k) == 0)
                for (int t = 0; t < k; t++) mark[i + t] = true;
        }
    }

    size_t copied = count(mark.begin(), mark.end(), true);
    double pc = original.empty() ? 0.0 : 100.0 * copied / original.size();
    int bars = static_cast<int>(pc / 100.0 * 40);
    cout << "\nCopied: " << fixed << setprecision(1) << pc << "% of the original\n";
    cout << "[" << string(bars, '#') << string(40 - bars, '-') << "]\n\n";
    cout << "Original text (copied parts highlighted):\n  ";
    for (size_t i = 0; i < original.size(); i++) {
        if (mark[i]) cout << clr::BG_YELLOW << clr::BLACK << original[i] << clr::RESET;
        else         cout << original[i];
    }
    cout << "\n" << defaultfloat;
}

// ======================================================================
// 4. LRU Cache
// ======================================================================
class LRUCache {
public:
    static constexpr size_t CAPACITY = 3;

    void request(const string& page) {
        auto it = pos_.find(page);
        if (it != pos_.end()) {
            order_.splice(order_.begin(), order_, it->second);  // move to newest
            ok("HIT: \"" + page + "\" found, moved to the newest position.");
            return;
        }
        if (order_.size() == CAPACITY) {
            string evicted = order_.back();
            order_.pop_back();
            pos_.erase(evicted);
            err("MISS: cache full. Oldest item \"" + evicted + "\" evicted.");
        } else {
            warn("MISS: \"" + page + "\" loaded into an empty slot.");
        }
        order_.push_front(page);
        pos_[page] = order_.begin();
    }

    void draw() const {
        cout << "\n";
        auto it = order_.begin();
        for (size_t i = 0; i < CAPACITY; i++) {
            if (it != order_.end()) {
                string text = *it;
                ++it;
                if (text.size() < 12) text.append(12 - text.size(), ' ');
                cout << clr::BG_BLUE << clr::WHITE << " " << text << " " << clr::RESET << "  ";
            } else {
                cout << clr::GRAY << "[ empty      ]" << clr::RESET << "  ";
            }
        }
        cout << "\n" << clr::GRAY << "newest  ->  oldest" << clr::RESET << "\n";
    }

private:
    list<string> order_;
    unordered_map<string, list<string>::iterator> pos_;
};

static void lruCache() {
    header("4. LRU Cache");
    info("Only 3 fast slots. When full, the least recently used item is evicted.");
    LRUCache cache;
    cache.draw();
    while (true) {
        string page = readLine("Open a page (Enter = back): ");
        if (page.empty()) return;
        cache.request(page);
        cache.draw();
    }
}

// ======================================================================
// 5. Break the Hash
// ======================================================================
static void breakTheHash() {
    header("5. Break the Hash");
    info("Pick a target box. Find a word that lands exactly there. 3 attempts.");
    int size = readInt("Table size 2-50 (Enter = 7): ");
    if (size < 2 || size > 50) size = 7;
    int target = readInt("Target box 0-" + to_string(size - 1) + " (Enter = 3): ");
    if (target < 0 || target >= size) target = min(3, size - 1);

    cout << "Find a word whose ASCII sum mod " << size << " = " << target << ".\n";
    int left = 3;
    while (left > 0) {
        string w = readLine("Attempts left " + to_string(left) + ". Your word: ");
        if (w.empty()) continue;
        --left;
        unsigned long long s = asciiSum(w);
        unsigned long long r = s % static_cast<unsigned long long>(size);
        cout << "\"" << w << "\": " << s << " mod " << size << " = " << r << "\n";
        if (r == static_cast<unsigned long long>(target)) {
            ok("You broke it! Collision at index " + to_string(target) + ".");
            return;
        }
        if (left > 0) err("Wrong box (" + to_string(r) + ").");
    }
    warn("Game over. Many words share the same index, that is why collisions are unavoidable.");
}

// ======================================================================
// Main menu
// ======================================================================
int main() {
    cout << clr::BOLD << clr::CYAN << "HASH VERSE VISUALIZER (console)" << clr::RESET << "\n";
    while (true) {
        cout << "\n 1) Memory Grid\n 2) Password Avalanche\n 3) Plagiarism Scanner\n"
             << " 4) LRU Cache\n 5) Break the Hash\n 0) Exit\n";
        int c = readInt("Choose: ");
        switch (c) {
            case 1: memoryGrid(); break;
            case 2: passwordAvalanche(); break;
            case 3: plagiarismScanner(); break;
            case 4: lruCache(); break;
            case 5: breakTheHash(); break;
            case 0: return 0;
            default: err("Invalid choice.");
        }
    }
    return 0;
}
