# FITExcel

## Overview

This C++ program written as a larger project for a C++ programming class implements some basic backend functionalities of an Excel-like table processor. 

## Features

- **Evaluate cell expressions**: The program uses an external library for parsing cell expressions to build abstract syntax trees representing the given expression that are then used for quick cell evaluation. The program can deal with various basic arithmetical/logical operations as well as absolute or relative references to other cells. It can also detect cycles in the expressions.
- **Use Excel-like coordinate system**: The program uses the coordinate system where numbers represent rows and uppercase letters represent columns.
- **Copy cells**: The program can copy rectangles of cells of any dimensions.
- **Save/Load**: The program can save and load spreadsheets as well as check whether the saved spreadsheet contents had been corrupted when loading.


