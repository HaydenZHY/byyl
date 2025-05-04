#!/bin/bash

pwd
rm lexer.exe

g++ Lex_Analysis.cpp -o lexer -std=c++11 -Wall -Wextra


./lexer < test.txt > output.txt