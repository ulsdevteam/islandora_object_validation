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

##### Configuring the CRON job
Even though th validation queue can be processed in sets directly from the configuration page, it may also be configured process that same number of objects by a CRON job call to the drush function included in this module.  To add this process to the CRON on the web server via command prompt:
```
$ sudo crontab -u apache -e
```
Enter these lines into the web server's CRON (edit to reference your site's web address for the uri parameter and optionally editing and including the MAILTO email address):
```
# Drush process a chunk of objects in validation queue.
# Optionally edit the following line to get an email digest of the validations that were processed.
# MAILTO=your_email@your_institution.edu
*/1 * * * *  /usr/bin/drush -u 1 islandora_object_validation_validate_chunk_of_objects --uri=http://digital.library.your_institution.edu --root=/var/www/html/drupal7/
```

## Usage
The "Validate objects" form can be used to add objects to the validation queue.  When validation is required on a large group of objects, the module uses a "validation queue" to process these items in a background process - when configured by CRON.

If you want to validate all of the objects in your repository, this form may be the easiest way to achieve this.  There are several approaches for getting objects into the validation queue.  
* entering all of the object's PID values into the selection box
* specifying one or more Solr queries
* selecting objects by Collection
* selecting objects by Model

#### Reports
The "Islandora Object Validation Report" and "Objects Queued for Validation" reports are handled using forena reports because the output can easily be exported as CSV.  To view the validation for any of the objects listed here, click the "Islandora PID" link to jump to the Validation History for that object (see next setion).

#### "Validation History" table on Islandora Object | Manage page. 
The "Validation History" tab should now appear on any Islandora object's Manage page if the user has "Administer Islandora Object Validation" permission.  From here, you can immediately perform a new validation test on any object.  Depending on the object state and the module configuration for that object's models, the record for the result of that test should appear in the table.

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
