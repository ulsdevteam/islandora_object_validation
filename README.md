# Islandora Object Validation

## Introduction

Checks Islandora objects for whether or not the object has all of the required datastreams.

## Requirements

* [Islandora](https://github.com/Islandora/islandora)
* [Forena](https://git.drupal.org/project/forena.git) | [ulsdevteam "Forena"](https://github.com/ulsdevteam/forena.git) *The ulsdevteam feature branch of Forena reports adds a Description value to the reports.  If the ulsdevteam branch of this module is installed or if the code is eventually merged into Forena, the descriptions of reports would be displayed -- else, only their Titles can be displayed.*

## Installation

Install as usual, see [this](https://drupal.org/documentation/install/modules-themes/modules-7) for further information.

Installing this module will also set up the following:
 - create the tables: islandora_object_validation_history, islandora_object_validation_queue
 - Deploy the Forena reports SQL and FRX files to the configured Forena locations.  This requires that the Drupal "**Private file system path**" is configured and editable by the web server (this path is configured at /admin/config/media/file-system).

## Permissions
Since this module uses the Forena reporting system, any users of this module should also have the separate permissions "Access Islandora Object Validation Reports Data" and "List reports" for the Forena reports.

## Configuration

Configure the datastreams that are required for validation for all object types at Administration » Islandora » Islandora Object Validation (admin/islandora/tools/islandora_object_validation). 


## Documentation

...

## Troubleshooting/Issues

Having problems or solved a problem? Check out the Islandora google groups for a solution.

* [Islandora Group](https://groups.google.com/forum/?hl=en&fromgroups#!forum/islandora)
* [Islandora Dev Group](https://groups.google.com/forum/?hl=en&fromgroups#!forum/islandora-dev)


## Maintainers/Sponsors

Current maintainers:

* [Willow Gillingham](https://github.com/bgilling)

## Author / License
Written by [Willow Gillingham](https://github.com/bgilling) for the [University of Pittsburgh](http://www.pitt.edu).  Copyright (c) University of Pittsburgh.

Released under a license of GPL v2 or later.
