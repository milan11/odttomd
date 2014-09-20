#  OdtToMd

ODT \(OpenDocument Text\) to MD \(Markdown\) converter\.

__License:__ Public domain\. Use at your own risk\. There is absolutely no warranty of any kind\.

##  Features

* paragraphs, line breaks inside a paragraph
* headings \(with automatic numbering\)
* lists \(multi\-level\)
* bold
* links

##  Hints

* to help OdtToMd understand the input file, prefer using styles for formatting – e\.g\. in LibreOffice:
  - Paramgraph Styles to create paragraphs and headings \(configure automatic heading numbering using Tools \-&gt; Outline Numbering\.\.\.\)
  - List Styles to create lists \(control levels using Demote One level and Promote One Level buttons on the Bullets and Numbering toolbar\)
  - Character Styles to control in\-line formatting \(e\.g\. create some style for the bold text\)

##  Build

* __implementation language:__ C, C\+\+11
* __platform:__ independent
* __latest source code:__ [https://github\.com/milan11/odttomd](https://github\.com/milan11/odttomd)
* __source code distribution:__ as CMake project
* __build dependencies:__
  - boost
  - expat
  - libzip
* __build steps:__
  - git clone git@github\.com:milan11/odttomd\.git
  - cd odttomd
  - cmake
  - make
* __possible CMake arguments:__
  - __debug build:__ \-DCMAKE\_BUILD\_TYPE=Debug
  - __force using gcc compiler:__ \-DCMAKE\_CXX\_COMPILER=g\+\+
  - __force using clang compiler:__ \-DCMAKE\_CXX\_COMPILER=clang\+\+

##  Supported Environment

* __OS:__ Linux tested only \(but should be multiplatform\)
* __needs these libraries \(is dynamically linked with\):__
  - expat, libzip

##  Usage

odttomd odt\_file &gt; md\_file

Non\-zero exit status means a fatal error \(invalid format, read / write error\)\.

Unsupported data of the input odt\_file is reported as a warning to standard error output \(and some defaults are used instead\) – this does not change the exit status\.

##  Examples

* the __examples directory__ contains MD files and their corresponding original ODT files
* these __repositories__ have their README\.MD files converted from README\.ODT files:
  - [https://github\.com/milan11/odttomd](https://github\.com/milan11/odttomd) \(this repository\)
  - [https://github\.com/milan11/repodbsync](https://github\.com/milan11/repodbsync)

