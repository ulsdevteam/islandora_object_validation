<?php

/**
 * @file
 * Admin callbacks for islandora_object_validation.
 */

/**
 * Admin form.
 */
function islandora_object_validation_reports(array $form, array &$form_state, $report_verb) {
  drupal_set_title(t('Validation reports - ' . $report_verb));
  $breadcrumb = array(
    l(t('Home'), '<front>'),
    l(t('Administration'), 'admin'),
    l(t('Islandora'), 'admin/islandora'),
    l(t('Islandora Object Validation'), 'admin/islandora/tools/islandora_object_validation'),
    t('Validation reports')
  );
  drupal_set_breadcrumb($breadcrumb);

  $filter_required_dsid = (is_array($_SESSION) && array_key_exists('required_dsid_' . $report_verb, $_SESSION)) ? $_SESSION['required_dsid_' . $report_verb] : '';

  $report_table = '<div><b>Report results</b><i>There are no matching records.</i></div>';
  $form = array(
    'report_verb' => array(
      '#type' => 'hidden',
      '#default_value' => $report_verb,
    ),
    'reports_wrapper' => array(
      '#type' => 'fieldset',
      '#attributes' => array(
        'class' => array('round_fieldset'),
      ),
      'link_report' => array(
        '#type' => 'item',
        '#title' => t('View results of object validation testing'),
        '#markup' => '<ul><li' . (($report_verb == 'failures') ? ' class="selected_link"' : '') . '>' . l(t('Validation Failures'), 'admin/islandora/tools/islandora_object_validation/reports/failures') .
          ' <span>- lists only the objects that fail the validation checking for that type of object</span></li>' .
          '<li' . (($report_verb == 'all') ? ' class="selected_link"' : '') . '>' . l(t('All records'), 'admin/islandora/tools/islandora_object_validation/reports/all') .
          ' <span>- lists all validation results.</span></li></ul>',
      ),
      'link_validations' => array(
        '#type' => 'item',
        '#title' => t('Validate objects'),
        '#markup' => '<ul><li>' .
          l(t('Validate Objects'), 'admin/islandora/tools/islandora_object_validation/validate_objects') .
          ' <span>- validate objects now</span></li></ul>',
      ),
    ),
    'islandora_pid' => array(
      '#type' => 'textfield',
      '#title' => t('Islandora PID'),
      '#default_value' => '',
    ),
    'required_dsid' => array(
      '#type' => 'textfield',
      '#title' => t('Required Datastream ID'),
      '#default_value' => $filter_required_dsid,
    ),
    'clearfilter' => array(
      '#type' => 'submit',
      '#value' => t('Clear'),
      '#weight' => 10,
    ),
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Filter results'),
      '#weight' => 10,
      '#suffix' => $report_table
    ),
  );

  return $form;
}

/**
 * Submit handler for admin form.
 */
function islandora_object_validation_reports_submit($form, &$form_state) {
  $report_verb = $form_state['values']['report_verb'];
  if ($form_state['triggering_element']['#value'] == t('Clear')) {
    $_SESSION['required_dsid_' . $report_verb] = '';
  }
  else {
    $filter_required_dsid = $form_state['values']['required_dsid'];
    $_SESSION['required_dsid_' . $report_verb] = $filter_required_dsid;

    dpm('submitted');
    dpm($form_state);
    dpm('report_verb');
    dpm($report_verb);
  }
}

