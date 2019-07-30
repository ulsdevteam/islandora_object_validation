<?php

/**
 * @file
 * Admin callbacks for islandora_object_validation.
 */

/**
 * Admin form.
 */
function islandora_object_validation_validate_objects(array $form, array &$form_state) {
  drupal_set_title(t('Validate objects'));
  $breadcrumb = array(
    l(t('Home'), '<front>'),
    l(t('Administration'), 'admin'),
    l(t('Islandora'), 'admin/islandora'),
    l(t('Islandora Object Validation'), 'admin/islandora/tools/islandora_object_validation'),
    t('Validate objects')
  );
  drupal_set_breadcrumb($breadcrumb);
  module_load_include('module', 'islandora_object_validation', 'islandora_object_validation');
  module_load_include('inc', 'islandora_object_validation', 'includes/utilities');

  $pids_attributes = array();
  $pids_fetch_method = isset($_SESSION['pids_fetch_method']) ? $_SESSION['pids_fetch_method'] : ISLANDORA_OBJECT_VALIDATION_FETCH_LISTPIDS;

  $pid_values = (isset($_SESSION['pids_value']) && $pids_fetch_method == ISLANDORA_OBJECT_VALIDATION_FETCH_LISTPIDS) ?
    json_decode($_SESSION['pids_value'], true) :
    array();
  // This will set the radio button to the appropriate choice for fetch method, and
  // the javascript will hide / show the appropriate form elements.
  if (isset($_SESSION['pids_value']) && $pids_fetch_method == ISLANDORA_OBJECT_VALIDATION_FETCH_LISTPIDS) {
    $form_state['input']['pids_fetch_method'] = $pids_fetch_method;
    $pids_attributes['readonly'] = 'readonly';
    $pids_attributes['disabled'] = 'disabled';
  }

  $default_pids_value = implode('
', $pid_values);
  $fetch_methods = islandora_object_validation_get_fetch_methods($pid_values);
  $tmp_model_options = islandora_object_validation_get_models();

  $extensions = array('xsl','xslt');

  $tmp_collections = islandora_object_validation_get_solr_options('RELS_EXT_hasModel_uri_ms:info\:fedora\/islandora\:collectionCModel', 'PID', 'fgs_label_s');
  asort($tmp_collections);
  $collections = array();
  foreach ($tmp_collections as $k => $collection) {
    $collections['info:fedora/' . $k] = $collection;
  }

  foreach ($tmp_model_options as $k => $model) {
    $model_options['info:fedora/' . $k] = $model;
  }

  $markup_export = '<p>' . t('This will allow a set of objects to be queued for ' .
    'validation.') . '</p>';

  $form['validation_form'] = array(
    'form_description' => array('#markup' => $markup_export),
    'pids' => array(
      '#type' => 'hidden',
      '#default_value' => $default_pids_value,
    ),
    'process_link' => array(
      '#type' => 'fieldset',
      '#title' => t('Process validation queue items'),
      '#attributes' => array(
        'class' => array('round_fieldset'),
      ),
      'cron_limit' => array(
        '#type' => 'item',
        '#description' => t('Clicking this will call `islandora_object_validation' .
          '_validate_chunk_of_objects` which is what gets called by the drush call ' .
          ' to "islandora_object_validation_validate_chunk_of_objects" via CRON ' .
          '(if configured). '),
        '#markup' => l(t('Process ' . number_format(variable_get('islandora_object_validation_cron_limit', 4000)) .
          ' items validation queue'), 'admin/islandora/tools/islandora_object_validation/queue_do_chunk'),
      ),
    ),
    'objectsformset' => array(
      '#type' => 'fieldset',
      '#title' => t('Select objects'),
      '#description' => t('For the export, select the method to select the objects.'),
      '#attributes' => array(
        'class' => array('round_fieldset'),
      ),
      'pids_fetch_method' => array(
        '#type' => 'radios',
        '#options' => $fetch_methods,
        '#default_value' => ((isset($form_state['input']['pids_fetch_method']) &&
          ($form_state['input']['pids_fetch_method']) <> '') ?
          $form_state['input']['pids_fetch_method'] : ''),
      ),
      'list_of_pids' => array(
        '#type' => 'textarea',
        '#title' => t('PID values'),
        '#attributes' => $pids_attributes,
        '#description' => t('PID values, one on each line.'),
        '#default_value' => $default_pids_value,
        '#group' => 'validation_form',
      ),
      'solr_query' => array(
        '#type' => 'textarea',
        '#title' => t('Solr query'),
        '#default_value' => (isset($form_state['input']['solr_query']) ? $form_state['input']['solr_query'] : $default_pids_value),
        '#description' => t('Use the query "q" section of any Solr query - ') .
          l(variable_get('islandora_solr_url', 'localhost:8080/solr'), variable_get('islandora_solr_url', 'localhost:8080/solr')) .
          t(' do not try to use filter queries "fq" or field lists "fl" or any special Solr options for this query.'),
        '#group' => 'validation_form',
      ),
      'collection' => array(
        '#type' => 'select',
        '#description' => t('Select collection'),
        '#title' => t('Collection'),
        '#options' => $collections,
        '#default_value' => (isset($form_state['input']['collection']) ? $form_state['input']['collection'] : $default_pids_value),
        '#size' => 10,
        '#group' => 'validation_form',
      ),
      'model' => array(
        '#type' => 'select',
        '#description' => t('Select model'),
        '#title' => t('Model'),
        '#options' => $model_options,
        '#default_value' => (isset($form_state['input']['model']) ? $form_state['input']['model'] : $default_pids_value),
        '#size' => 7,
        '#group' => 'validation_form',
      ),
    ),
    'utility_wrapper' => array(
      '#type' => 'fieldset',
      '#attributes' => array(
        'class' => array('round_fieldset'),
      ),
      'solr_limit' => array(
        '#type' => 'textfield',
        '#title' => t('Solr query limit'),
        '#description' => t('Use to control the maximum number of Solr records returned for "Solr query", "Collection", "Model" fetch methods.'),
        '#default_value' => variable_get('islandora_object_validation_solr_limit', 10000),
      ),
    ),
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Add to Queue'),
    ),
  );

  return $form;
}

/**
 * Submit handler for admin form.
 */
function islandora_object_validation_validate_objects_submit($form, &$form_state) {
  module_load_include('inc', 'islandora_object_validation', 'includes/utilities');
  global $user;

  $pids_fetch_method = $form_state['values']['pids_fetch_method'];
  $limit = $form_state['values']['solr_limit'];
  $pids = NULL;

  // Unset the _SESSION value here.
  if (isset($_SESSION['pids_value']) && isset($_SESSION['pids_fetch_method'])) {
    unset($_SESSION['pids_value']);
    unset($_SESSION['pids_fetch_method']);
  }

  // Depending on the pids_fetch_method value, the PID values are derived one of 4 different ways:
  switch ($pids_fetch_method) {
    // 0 - Solr query
    case ISLANDORA_OBJECT_VALIDATION_FETCH_SOLR:
      $solr_query = $form_state['values']['solr_query'];
      drupal_set_message(t('Lookup PIDS for Solr query: "') . $solr_query . '"');
      $pids = islandora_object_validation_get_pids_solr_query($solr_query, $limit);
      break;

    // 1 - List of PIDS
    case ISLANDORA_OBJECT_VALIDATION_FETCH_LISTPIDS:
      $pids = $form_state['values']['list_of_pids'];
      break;

    // 2 - Collection
    case ISLANDORA_OBJECT_VALIDATION_FETCH_COLLECTION:
      $collection = $form_state['values']['collection'];
      drupal_set_message(t('Lookup PIDS for Collection: "') . $collection . '"');
      $pids = islandora_object_validation_get_pids_collection($collection, $limit);
      break;

    // 3 - Model
    case ISLANDORA_OBJECT_VALIDATION_FETCH_MODEL:
      $model = $form_state['values']['model'];
      drupal_set_message(t('Lookup PIDS for fedora Model: "') . $model . '"');
      $pids = islandora_object_validation_get_pids_model($model, $limit);
      break;
  }
  if ($pids) {
    $bad_pids = $skipped = array();
    $added_count = 0;
    $pids_arr = explode("\n", $pids);
    foreach ($pids_arr as $pid) {
      $pid = trim($pid);
      $islandora_object = islandora_object_load($pid);
      if (is_object($islandora_object)) {
        if (islandora_object_validation_queue_validate_object($islandora_object)) {
          $added_count++;
        }
        else {
          $skipped[] = $pid;
        }
      }
      else {
        $bad_pids[] = $pid;
      }
    }
    if (count($skipped) > 0) {
      drupal_set_message(t('At least one of datastream for the objects could not be updated.  These are: \'') . implode("', '", $relationship_skipped) . '\'', 'info');
    }
    if (count($bad_pids) > 0) {
      drupal_set_message(t('At least one of the PID values were not able to be loaded as an object.  These are: \'') . implode("', '", $bad_pids) . '\'', 'warning');
    }
    drupal_set_message(number_format($added_count) . t(' objects have been added to the validation queue.'));
  }
  else {
    drupal_set_message(t('There were no PID values for this export.  Please try again.'));
  }
}

/**
 * This will call the function to add a record to the queue table
 * (islandora_object_validation_queue) which is used to processes validations
 * out-of-process (configured to be called via CRON).
 *
 * @param type $islandora_object
 */
function islandora_object_validation_queue_validate_object($islandora_object) {
  module_load_include('inc', 'islandora_object_validation', 'includes/db');
  return islandora_object_validation_add_queue_record($islandora_object);
}

/**
 * This will call the function to add a record to the queue table
 * (islandora_object_validation_queue) which is used to processes validations
 * out-of-process (configured to be called via CRON).
 *
 * @param type $islandora_object
 */
function islandora_object_validation_queue_do_chunk() {
  module_load_include('inc', 'islandora_object_validation', 'islandora_object_validation.drush');
  islandora_object_validation_validate_chunk_of_objects();
  drupal_goto('admin/islandora/tools/islandora_object_validation/validate_objects');
}

function islandora_object_validation_queue_object($pid) {
  $islandora_object = islandora_object_load($pid);
  module_load_include('inc', 'islandora_object_validation', 'includes/db');
  islandora_object_validation_add_queue_record($islandora_object);
  drupal_set_message(t('The object @pid has been added to the validation queue.'),
      array('@pid' => $pid), 'status');
  drupal_goto('islandora/object/' . $pid . '/manage/islandora_object_validation');
}
