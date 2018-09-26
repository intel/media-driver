# Intel(R) Media Driver Docker Image


## Introduction

Docker files in the child folders are intended to provide latest validated media stack for operating systems disclosed in a folder names. Thus, you can see that media stack ingredients are checked out by specific revision, not by the latest head.


## Prerequisites

Install docker environment in your machine.


## Docker Image Tag

Latest tag is based on Ubuntu/18.04/Dockerfile. Ubuntu 16.04/Fedora28 images are in plan which are located under dockers/<OS>/<version>. You can submit issue to request other docker images you need.

Starting from 18.3 release branch, Release-<branch_name>_<base_image> tag are also avilable for download.

You can find detail tags from [hub.docker/intelmediadriver](https://hub.docker.com/r/intelmediadriver/media-driver/tags/).


## Build Dockerfile

To customize the docker image, you can build it like below after changing Dockerfile. 
```
$ cd Tools/dockers/<OS>/<version>
$ docker build -t test:v1 .
```
