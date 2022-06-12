/*
 *   SMPRSA - Simple RSA is an encrpytion software. It is meant as a demonstration/learning tool. NOT FOR REAL USE CASE.
 *   Copyright (C) 2022 Arhum Z. Nayyar
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
*/

// SMPRSA.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <iostream>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/random.hpp>
#include <random>
#include <fstream>
#include <sstream>
#include <chrono>

using namespace boost::multiprecision;
using namespace boost::random;

#define CPP_INT_MAX ((cpp_int) - 1)

cpp_int euclidean(cpp_int a, cpp_int b) {
    if (a == 0)
        return b;
    return euclidean(b % a, a);
}

cpp_int extendedEuclidean(cpp_int a, cpp_int b, cpp_int& x, cpp_int& y) {
    if (b == 0) {
        x = 1;
        y = 0;
        return a;
    }
    cpp_int x1{ "" }, y1{ "" };
    cpp_int d{ "" };
    d = extendedEuclidean(b, a % b, x1, y1);
    x = y1;
    y = x1 - y1 * (a / b);
    return d;
}

cpp_int inverse(cpp_int a, cpp_int m) {
    cpp_int x{ "" }, y{ "" };
    cpp_int gcd{ "" };
    gcd = extendedEuclidean(a, m, x, y);
    if (gcd != 1) {
        return x;
    }
    else {
        x = (x % m + m) % m;
    }
    return x;
}

cpp_int modExpo(cpp_int a, cpp_int b, cpp_int p) {
    // Memory-efficent method to calculate (A * B) % P (Fermats).
    // Possible overflow with extremely large numbers.
    cpp_int result{ "0" };
    //result = 0;

    if (a == 0)
        return 0;

    if (CPP_INT_MAX / a > b)
        return (a * b) % p;

    //else {
    //    cpp_int a1{ "1" };
    //    cpp_int a2{ "1" };

    //    a1 = a / 2;
    //    a2 = a - a1;

    //   return result;//(modExpo(a1, b, p) + modExpo(a2, b, p)) % p;
    //}

    a = a % p;
    while (b > 0) {
        if (b & 1) {
            result = (result + a) % p;
        }
        a = (2 * a) % p;
        b /= 2;
    }
    return result % p;
}

cpp_int powermodSM(cpp_int base, cpp_int exp, cpp_int p) {
    cpp_int ans = 1;//{ "1" };
    //ans = 1;

    //base = base % p;

    while (exp > 0) {
        // Compare by bits. See placement of ones if 1.
        if (exp & 1) {
            ans = modExpo(ans, base, p);
        }
        // Perform multiplication and then shift the binary to the right.
        base = modExpo(base, base, p);
        exp >>= 1;
    }
    return ans % p; //return ans;
}

bool checkComp(cpp_int p, cpp_int n, cpp_int pOdd, cpp_int exp) {
    // Check to see if number is a likely-prime number.
    // The number chosen by random, n, rasied to the odd p value calculated should be -1 % p
    cpp_int remain{ "1" };
    remain = powermodSM(n, pOdd, p);

    // Corner case. Very likely-prime.
    if (remain == p - 1 || remain == 1) {
        return false;
    }

    // Need to check for liars. Is n^((2^i)pOdd) == -1 (mod p) for 1 <= i <= exp-1?
    for (int i = 1; i < exp; i++) {
        remain = modExpo(remain, remain, p);
        if (remain == p - 1) {
            return false;
        }
    }
    return true;
}

bool checkPrime(cpp_int p, int s) {
    // 1 or p is even, return false (not prime)
    if (p == 1 || p % 2 == 0)
        return false;
    // 2 is the only even prime number, just return true (sanity check).
    // 3 is a corner case just like with the above. Rest of primes are OK to check.
    if (p == 2 || p == 3)
        return true;

    // p - 1 to get an even number
    cpp_int pOdd{ "1" };
    pOdd = p - 1;
    cpp_int exp{ "1" };
    exp = 0; // Exponent
// d & 1 - compare by bit. Check that the bit is set to 1.  
    while ((pOdd & 1) == 0) {
        pOdd >>= 1;
        exp++;
    }

    for (int i = 0; i <= s; ++i) {
        // Choose a random number n in the set of (1,2,..., p-1). 
        // As we divide p-1 by p we should g et a remainder of 1 or 0 (Fermats).
        // And that would mean it is prime.
        cpp_int n{ "1" };
        n = 2 + rand() % (p - 2);
        // Perform a check to see if it is composite.
        if (checkComp(p, n, pOdd, exp)) {
            return false;
        }
    }
    return true;
}

cpp_int calcE(cpp_int e, cpp_int phiN) {

    //std::random_device rd; //Seed for the random number
    //std::mt19937 gen(rd()); //Mersenne Twister Engine with rd seed ^
    //std::uniform_int_distribution<> luckyNumber(1, 2147483647);//2147483647); 
    mt19937 mt;
    uniform_int_distribution<cpp_int> ui((cpp_int(1) << 64), cpp_int(1) << 256);
    cpp_int random{ "" };
    random = ui(mt);

    std::cout << random << std::endl;
    e = random;

    if (euclidean(e, phiN) == 1) {
        std::cout << "gcd = 1 " << e << " , phiN = " << phiN << std::endl;
    }
    else {
        while (euclidean(e, phiN) != 1) {
            e = ui(mt);
        }
        std::cout << "gcd = 1 for new rerolled e: " << e << " , phiN = " << phiN << std::endl;
    }
    return e;
}

void generateKey(bool randomMode, cpp_int p, cpp_int q, cpp_int msg) {
    mt11213b base_gen(clock());
    independent_bits_engine<mt11213b, 512, cpp_int> gen(base_gen);

    int s = 12; // s also known as k, is = 12 - Which satisfies 512-bit.

    if (randomMode) {
        p = gen();
        q = gen();
    }

    std::cout << "p = " << p << " | Checking if prime" << std::endl;
    if (checkPrime(p, s)) {
        std::cout << "Yes..." << p << " is prime (p)" << std::endl;
    }

    else {
        std::cout << "No..." << "p is not prime." << std::endl;
        if (randomMode) {
            while (!checkPrime(p, s)) {
                std::cout << "Checking new p: " << p << std::endl;
                p = gen();
            }
            std::cout << "p is finally prime p = " << p << std::endl;
        }
        else {
            std::cout << "Try again and ensure you are entering a prime number for p." << std::endl;
            return;
        }
    }

    std::cout << "q = " << q << " | Checking if prime" << std::endl;
    if (checkPrime(q, s)) {
        std::cout << "Yes..." << q << " is prime (q)" << std::endl;
    }

    else {
        std::cout << "No..." << q << " is not prime." << std::endl;
        if (randomMode) {
            while (!checkPrime(q, s)) {
                q = gen();
                std::cout << "new q = " << q << std::endl;
            }
            std::cout << "q is finally prime q = " << q << std::endl;
        }
        else {
            std::cout << "Try again and ensure you are entering a prime number for q." << std::endl;
            return;
        }
    }

    cpp_int n{ "1" };
    n = p * q;
    cpp_int phiN{ "1" };
    phiN = (p - 1) * (q - 1);
    cpp_int e{ "1" };
    e = 1;
    cpp_int d{ "1" };
    d = 1;

    e = calcE(e, phiN);

    int counter = 0;
    while (d <= 153) {
        if (counter > 500) {
            std::cout << "\n\nExceeded 500 attempts to obtain a value for d.\nPlease try again!" << std::endl;
            return;
        }
        d = inverse(e, phiN);
        if (d <= 153) { // Using if statement just in case the first time d is above 153 we dont run calcE again and waste time!
            std::cout << "it is lower than 153! Recalculating e" << std::endl;
            e = calcE(e, phiN);
            counter++;
        }
    }

    std::cout << "Key Public: n = " << n << " , e = " << e << std::endl;
    std::cout << "Key Private: d = " << d << std::endl;

    cpp_int encrypted = powermodSM(msg, e, n);

    std::cout << "Plaintext: " << msg << std::endl;
    std::cout << "Encrypted: " << encrypted << std::endl;

    cpp_int decrypted = powermodSM(encrypted, d, n);

    std::cout << "Decrypted: " << decrypted << std::endl;

    std::cout << "Outputting Key to Key.txt" << std::endl;

    std::ofstream myfile;
    myfile.open("Key.txt");
    myfile << d << "\n" << n;
    myfile.close();

    myfile.open("Cipher.txt");
    myfile << encrypted << std::endl;
    myfile.close();

    std::cout << "Sucessfully written the Key to Key.txt and the encrypted text to Cipher.txt" << std::endl;
}

void decryption() {
    std::string filename, line;
    std::ifstream data;
    cpp_int ciphertext{ "1" }, d{ "1" }, n{ "1" }, e{ "1" }, key{ "1" };

    data.open("Key.txt");
    if (data.good()) {
        if (data.peek() != data.eof()) {
            while (data >> d >> n) {
                std::cout << "Added key" << std::endl;
            }
        }
    }
    data.close();

    data.open("Cipher.txt");
    if (data.good()) {
        if (data.peek() != data.eof()) {
            while (data >> ciphertext) {
                std::cout << "Added cipher text" << std::endl;
            }
        }
    }
    data.close();

    std::cout << "ciphertext: " << ciphertext << std::endl;
    std::cout << "d: " << d << std::endl;
    std::cout << "n: " << n << std::endl;

    cpp_int decrypted = powermodSM(ciphertext, d, n);
    std::cout << "Decrypted: " << decrypted << std::endl;
}

int main() {
    bool run = true;
    while (run) {
        std::random_device rd; //Seed for the random number
        std::mt19937 gen(rd()); //Mersenne Twister Engine with rd seed ^
        std::uniform_int_distribution<> luckyNumber(1, 10000);//2147483647);
        cpp_int p{ "17" };
        cpp_int q{ "17" };
        bool randomMode = false;
        int answer;
        cpp_int msg{ "25" };
        char yn = 'a';
        msg = luckyNumber(gen);

        std::cout << "Select an operation to perform\n1. Encryption\n2. Decryption\n3. Just show me a demo!" << std::endl;
        std::cin >> answer;
        auto start = std::chrono::high_resolution_clock::now();
        if (answer == 1) {

            std::cout << "Enter the text(digits only) you would like to encrypt." << std::endl;
            std::cin >> msg;

            std::cout << "Use random values for p and q? (y/n)" << std::endl;
            std::cin >> yn;

            if (yn == 'y' || yn == 'Y') {
                randomMode = true;
                generateKey(randomMode, p, q, msg);
            }
            else if (yn == 'n' || yn == 'N') {
                std::cout << "Enter a prime number for p" << std::endl;
                std::cin >> p;

                std::cout << "Enter a prime number for q" << std::endl;
                std::cin >> q;

                generateKey(randomMode, p, q, msg);
            }
            else {
                std::cout << "Invalid input." << std::endl;
                return 0;
            }
        }
        else if (answer == 2) {
            decryption();
        }
        else if (answer == 3) {
            randomMode = true;
            generateKey(true, p, q, msg);
        }
        else {
            std::cout << "Invalid input." << std::endl;
            return 0;
        }

        auto stop = std::chrono::high_resolution_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(stop - start);
        std::cout << "\nCompleted in: " << elapsedTime.count() << " seconds" << std::endl;

        std::cout << "\nRun the program again? (y/n)" << std::endl;
        std::cin >> yn;

        if (yn == 'y' || yn == 'Y') {
            system("CLS");
            continue;
        }
        else if (yn == 'n' || yn == 'N') {
            run = false;
            return 0;
        }
        else {
            std::cout << "Invalid input." << std::endl;
            return 0;
        }
    }
    return 0;
}

