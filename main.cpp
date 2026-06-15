/**
 * @gobode_lab
 * @file mini_dns_resolver.cpp
 * @author Eduardo
 * @brief Minimal DNS resolver pipeline:
 *        INPUT(domain) → RESOLVE → EXTRACT IPv4 → FORMAT(binary, hex)
 * @date 2026
 */

#include <iostream>
#include <string>
#include <vector>
#include <bitset>
#include <iomanip>
#include <arpa/inet.h>
#include <netdb.h>

/* ---------------------------------------------------------
 * PHASE 1 — Acquire domain input
 * Why: isolate user input from resolver logic
 * --------------------------------------------------------- */
std::string acquire_domain() {
    std::string domain;
    std::cout << "Domain: ";
    std::cin >> domain;
    return domain;
}

/* ---------------------------------------------------------
 * PHASE 2 — Resolve domain using getaddrinfo (modern API)
 * Why: deterministic, thread‑safe, IPv6‑aware
 * --------------------------------------------------------- */
bool resolve_domain(const std::string& domain, in_addr& out_addr) {
    addrinfo hints{};
    hints.ai_family = AF_INET;      // Force IPv4
    hints.ai_socktype = SOCK_STREAM;

    addrinfo* result = nullptr;
    int status = getaddrinfo(domain.c_str(), nullptr, &hints, &result);

    if (status != 0 || !result) return false;

    sockaddr_in* ipv4 = reinterpret_cast<sockaddr_in*>(result->ai_addr);
    out_addr = ipv4->sin_addr;

    freeaddrinfo(result);
    return true;
}

/* ---------------------------------------------------------
 * PHASE 3 — Convert IPv4 to binary segments
 * Why: explicit byte‑level visibility
 * --------------------------------------------------------- */
std::vector<std::bitset<8>> to_binary(const in_addr& addr) {
    std::vector<std::bitset<8>> bins;
    const unsigned char* bytes =
        reinterpret_cast<const unsigned char*>(&addr);

    for (int i = 0; i < 4; ++i)
        bins.emplace_back(bytes[i]);

    return bins;
}

/* ---------------------------------------------------------
 * PHASE 4 — Convert IPv4 to hex segments
 * Why: low‑level representation for debugging / RE
 * --------------------------------------------------------- */
std::vector<std::string> to_hex(const in_addr& addr) {
    std::vector<std::string> hexes;
    const unsigned char* bytes =
        reinterpret_cast<const unsigned char*>(&addr);

    for (int i = 0; i < 4; ++i) {
        std::stringstream ss;
        ss << std::hex << std::setw(2) << std::setfill('0')
           << static_cast<int>(bytes[i]);
        hexes.push_back(ss.str());
    }
    return hexes;
}

/* ---------------------------------------------------------
 * PHASE 5 — Display pipeline output
 * Why: clean separation of computation vs presentation
 * --------------------------------------------------------- */
void display_output(const std::string& ip,
                    const std::vector<std::bitset<8>>& bins,
                    const std::vector<std::string>& hexes) {

    std::cout << "Resolved IPv4: " << ip << "\n";

    std::cout << "Binary: ";
    for (size_t i = 0; i < bins.size(); ++i) {
        std::cout << bins[i];
        if (i != bins.size() - 1) std::cout << ".";
    }
    std::cout << "\n";

    std::cout << "Hex: ";
    for (size_t i = 0; i < hexes.size(); ++i) {
        std::cout << hexes[i];
        if (i != hexes.size() - 1) std::cout << ":";
    }
    std::cout << "\n";
}

/* ---------------------------------------------------------
 * MAIN — Pipeline execution
 * --------------------------------------------------------- */
int main() {
    std::string domain = acquire_domain();

    in_addr addr{};
    if (!resolve_domain(domain, addr)) {
        std::cerr << "Resolution failed.\n";
        return 1;
    }

    std::string ip_str = inet_ntoa(addr);
    auto bins = to_binary(addr);
    auto hexes = to_hex(addr);

    display_output(ip_str, bins, hexes);
    return 0;
}
