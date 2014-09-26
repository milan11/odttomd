#  OdtToMd

ODT \(OpenDocument Text\) to MD \(Markdown\) converter.

**License:** Public domain. Use at your own risk. There is absolutely no warranty of any kind.

##  Features

* paragraphs, line breaks inside a paragraph
* headings \(with automatic numbering\)
* bullet lists and numbered lists \(multi\-level\)
* bold
* links

##  Hints

* to help OdtToMd understand the input file, prefer using styles for formatting – e.g. in LibreOffice:
  - Paramgraph Styles to create paragraphs and headings \(configure automatic heading numbering using Tools \-&gt; Outline Numbering...\)
  - List Styles to create lists \(control levels using Demote One level and Promote One Level buttons on the Bullets and Numbering toolbar\)
  - Character Styles to control in\-line formatting \(e.g. create some style for the bold text\)

##  Build

* **implementation language:** C, C\+\+11
* **platform:** independent
* **latest source code:** [https://github.com/milan11/odttomd](https://github.com/milan11/odttomd)
* **source code distribution:** as CMake project
* **build dependencies:**
  - boost
  - expat
  - libzip
* **build steps:**
  - git clone git@github.com:milan11/odttomd.git
  - cd odttomd
  - cmake
  - make
* **possible CMake arguments:**
  - **debug build:** \-DCMAKE\_BUILD\_TYPE=Debug
  - **force using gcc compiler:** \-DCMAKE\_CXX\_COMPILER=g\+\+
  - **force using clang compiler:** \-DCMAKE\_CXX\_COMPILER=clang\+\+

##  Supported Environment

* **OS:** Linux tested only \(but should be multiplatform\)
* **needs these libraries \(is dynamically linked with\):**
  - expat
  - libzip

##  Usage

###  Simple

odttomd odt\_file &gt; md\_file

###  Output types

Use \-\-profile option to specify output type:

* **standard** – standard Markdown format
* **github** – GitHub Flavored Markdown
* **plain\_text** – without Markdown marks and not usable as Markdown \(e.g. tries to preserve numbering formats from ODT, but some of these formats cannot be understood by Markdown parsers\)
* **readable** – more human\-readable when opening as text
* **parseable** – uses more straightforward rules which allows simpler parsing

###  Error reporting

Non\-zero exit status means a fatal error \(invalid format, read / write error\).

Unsupported data of the input odt\_file is reported as a warning to standard error output \(and some defaults are used instead\) – this does not change the exit status.

##  Examples

* the **examples directory** contains MD files \(for each output type\) and their corresponding original ODT files
* these **repositories** have their README.MD files converted from README.ODT files:
  - [https://github.com/milan11/odttomd](https://github.com/milan11/odttomd) \(this repository\)
  - [https://github.com/milan11/repodbsync](https://github.com/milan11/repodbsync)

