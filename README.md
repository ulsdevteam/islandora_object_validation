# Islandora Object Validation

## Introduction

Sometimes some objects can be partially ingested or otherwise corrupted by some other failure.  In cases where this happens, the Islandora Object Validation utility is a tool designed to test Islandora objects for whether or not they have all of the required datastreams and optionally to assert that relationships exist to other objects.

This module does nothing to fix object that lack a required datastream.  In most cases, the source files must be ingested again.  In some cases, other utilities can be used to push datastreams *en-masse* onto objects that need them such as the Islandora Datastreams I/O utility.

## Requirements

* [Islandora](https://github.com/Islandora/islandora)
* [Forena](https://git.drupal.org/project/forena.git) | [ulsdevteam "Forena"](https://github.com/ulsdevteam/forena.git) *The ulsdevteam feature branch of Forena reports adds a Description value to the reports.  If the ulsdevteam branch of this module is installed or if the code is eventually merged into Forena, the descriptions of reports would be displayed -- else, only their Titles can be displayed.*

## Installation

Install as usual, see [this](https://drupal.org/documentation/install/modules-themes/modules-7) for further information.

Installing this module will also set up the following:
 - create the tables: islandora_object_validation_history, islandora_object_validation_queue
 - Deploy the Forena reports SQL and FRX files to the configured Forena locations.  This requires that the Drupal "**Private file system path**" is configured and editable by the web server (this path is configured at /admin/config/media/file-system).

## Permissions
"Administer Islandora Object Validation" permission is required to configure and use this module.  Since this module uses the Forena reporting system, any users of this module should also have the separate permissions "Access Islandora Object Validation Reports Data" and "List reports" for the Forena reports.

## Configuration
Configuration for the module can be found at Administration » Islandora » Islandora Object Validation (admin/islandora/tools/islandora_object_validation).  
**Solr query limit** will limit the number of results that can come back from Solr queries behind the selection of objects for the "Validate objects" form.
**CRON limit** will control how many objects are tested during each run of the CRON process to validate objects.  This may impact performance -- and it would be better to process fewer items more frequently than to process thousands on a daily basis.

**Islandora model requirements table** for all object models, configure which datastreams are required for validation and optionally they may assert that objects of this type must have a given relationship to another object that is of a given type.

## Usage
The "Validate objects" form can be used to add objects to the validation queue.

#### "Validation History" table on Islandora Object | Manage page. 
After installation, a "Validation History" tab should appear on any Islandora object's Manage page if the user has "Administer Islandora Object Validation" permission.

#### Reports
The one report is almost the same as what appears on each objects' Validation History tab, but these forena reports can be used to export the results as CSV.

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
