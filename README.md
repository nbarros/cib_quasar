# cib_quasar

CIB SC server using Quasar framework

## Contacts

* Nuno Barros <barros@lip.pt>

## Release notes

### 3.1.0

* Fixed pending bugs causing the firing segments to fail to initialize or stop too early
  * Now using the motor position for the initial movement assessment, with speed checks when movement is reaching destination
* Fixed position syncing on the CIB regardless of the motor moving or not

### 3.0.0

* Preliminary production release. 
* Fixes several bugs and streamlines the operation by simplifying the pause, standby and resume operations

## Changelog

* 24/11/2024: Nuno Barros <barros@lip.pt>
  * Fixed issues with trace start and end.
  * Tagging version 3.1.0  
