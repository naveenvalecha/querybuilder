<?php

/**
 * @file
 * Optimise database.
 */

// Load querybuilder_common file.
module_load_include('inc', 'querybuilder', 'includes/querybuilder_common');

/**
 * Form builder; Return form for clear cache.
 */
function querybuilder_optimise($form, &$form_state) {
	$form['optimise'] = array(
	 '#type' => 'checkbox',
	 '#title' => t('Optimise database'),	
	 '#description' => t('When checked, optimise database every time when cron run.'),	
	);
 
 	$form['submit'] = array(
	 '#type' => 'submit',
	 '#value' => t('Save'),
	);
  return $form;
}

/**
 * Process result from querybuilder optimise form.
 */
function querybuilder_optimise_submit($form, $form_state) {
  $optimise = $form_state['values']['optimise'];
  if ($optimise) {
  	variable_set['clear_cache_by_cron'] = TRUE;
  }
  else {
    variable_set['clear_cache_by_cron'] = FALSE;
  }
	return;
}

/**
 * Implements hook_cron().
 */
function querybuilder_cron() {
  $clear_cache = variable_get['clear_cache_by_cron'];
  if (!empty($clear_cache)) {
  	drupal_flush_all_caches();
  }
}
