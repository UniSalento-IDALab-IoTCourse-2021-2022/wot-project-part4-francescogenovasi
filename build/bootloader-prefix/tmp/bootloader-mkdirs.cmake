# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/Users/francescogenovasi/esp-idf/components/bootloader/subproject"
  "/Users/francescogenovasi/CLionProjects/wot-project-part4-mesh_network_provisioner-dinoi-genovasi/build/bootloader"
  "/Users/francescogenovasi/CLionProjects/wot-project-part4-mesh_network_provisioner-dinoi-genovasi/build/bootloader-prefix"
  "/Users/francescogenovasi/CLionProjects/wot-project-part4-mesh_network_provisioner-dinoi-genovasi/build/bootloader-prefix/tmp"
  "/Users/francescogenovasi/CLionProjects/wot-project-part4-mesh_network_provisioner-dinoi-genovasi/build/bootloader-prefix/src/bootloader-stamp"
  "/Users/francescogenovasi/CLionProjects/wot-project-part4-mesh_network_provisioner-dinoi-genovasi/build/bootloader-prefix/src"
  "/Users/francescogenovasi/CLionProjects/wot-project-part4-mesh_network_provisioner-dinoi-genovasi/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/francescogenovasi/CLionProjects/wot-project-part4-mesh_network_provisioner-dinoi-genovasi/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
