<?php

/**
 * @file
 * querybuilder_insert.inc
 *
 * Demonstrates for dynamic insert query.
 */

// Load querybuilder_common file.
module_load_include('inc', 'querybuilder', 'includes/querybuilder_common');

/**
 * Implements querybuilder_insert
 *   A form for insert query.
 *
 * @param $form
 * @param $form_state
 *
 * @return array
 */
function querybuilder_insert($form, &$form_state) {
  // Get list of tables name from database.
	$tbl_list = querybuilder_get_tables();
	
	// Get value from $form_state['values']
	$sel_qry_table = isset($form_state['values']['query_table']) ? $form_state['values']['query_table'] : '_none';
	$sel_qry_fields = isset($form_state['values']['query_fields']) ? $form_state['values']['query_fields'] : '_none';
  
	if (!empty($sel_qry_table) && $sel_qry_table != '_none') {
		// Get list of field name from table.
		$query_fields = _querybuilder_get_fields($sel_qry_table);
		$query_fields_list = array_combine($query_fields, $query_fields);
  }  
	$query_fields_list['_none'] = "Select Fields";

	$form['query_table'] = array(
    '#type' => 'select', 
    '#title' => 'Select Table',
    '#options' => $tbl_list,
    '#default_value' => '_none',
    '#ajax' => array(
      'callback' => 'querybuilder_querytable_callback',
      'wrapper' => 'query-table-fields',
     )
  );
  
  $form['query_fields'] = array(
    '#type' => 'select', 
    '#title' => 'Select Fields',
    '#multiple' => TRUE, 
		'#default_value' => '_none',
    '#options' => $query_fields_list,
    '#prefix' => '<div id="query-table-fields">',
    '#suffix' => '</div>',
    '#ajax' => array(
      'callback' => 'querybuilder_queryfield_callback',
      'wrapper' => 'placeholder-fields',
      )
  );

  $form['placeholder'] = array(
    '#type' => 'hidden',
    '#prefix' => '<div id="placeholder-fields">',
    '#suffix' => '</div>',
    '#tree' => TRUE,
  );
	
  $map_fields = array();
  if (!empty($sel_qry_fields) && isset($form_state['values']['query_fields'])) {
    if (!array_key_exists('_none', $sel_qry_fields)) {
			$form['placeholder']['#type'] = 'fieldset';
			foreach ($query_fields as $key => $value) {
				if (in_array($value, $sel_qry_fields)) {
					if ($key == "blob") {
						 $form['placeholder'][$value] = array(
							'#type' => 'textfield', 
							'#title' => 'Value for ' . $value,
							'#attributes' => array('disabled' => TRUE),
							'#description' => 'Field Type ' . $key,
						);
					}
					else {
						 $map_fields[$value] = $key;
						 $form['placeholder'][$value] = array(
							'#type' => 'textfield', 
							'#title' => 'Value for ' . $value,
							'#description' => 'Field Type ' . $key,
						);
					}
				}
			}
			$form_state['fields'] = $map_fields;
		}
  }

	$form['build_query'] = array(
	 '#type' => 'checkbox',
	 '#title' => t('Build Query'),
		'#ajax' => array(
			'callback' => 'querybuilder_buildquery_callback',
			'wrapper' => 'textfields',
			'effect' => 'fade',
		),
	);
 
  $form['fieldsets'] = array(
    '#title' => t("Output"),
    '#prefix' => '<div id="textfields">',
    '#suffix' => '</div>',
    '#type' => 'fieldset',
  );

	if (!empty($form_state['values']['build_query']) && $form_state['values']['build_query']) {    
		$form['fieldsets']['query_outputs'] = array(
			'#title' => 'Query and Its output',
			'#markup' => querybuilder_insert_result($form, $form_state),
			'#prefix' => '<div id="query-output">',
			'#suffix' => '</div>',
		);
	}
  return $form;
}

/**
 * Implements querybuilder_insert_result
 *   Build query from user input.
 *
 * @param $form
 * @param $form_state
 *
 * @return string
 *   HTML string output
 */
function querybuilder_insert_result($form, $form_state) {
  $query_table = $form_state['values']['query_table'];
  $map_fields_data = array();
	$find= array('(', ')');  
  if (isset($form_state['fields']) && !empty($form_state['fields'])) {
		$query_fields = $form_state['fields'];
		foreach ($query_fields as $key => $value) {
			// Find data type and its length.
			$tmp_array = array();
			$temp_str = str_replace($find, ' ', $value);
			$tmp_array = explode(' ', $temp_str);
			$data_type = $tmp_array[0];
			$length = $tmp_array[1];
			if (isset($form_state['values']['placeholder'][$key]) && !empty($form_state['values']['placeholder'][$key])) {
				$field_value = check_plain($form_state['values']['placeholder'][$key]);
				$validate_result = FALSE;
				// Validate user-input.
				$validate_result = validate_value($data_type, $length, $field_value); 
				if ($validate_result) {
					$map_fields_data[$key] = $field_value;
				}
      }				
		}
    if (!empty($map_fields_data)) {		
			$act_query = '$transaction = db_transaction();';
			$act_query .= "<br>" . 'try {';
			$act_query .= "<br>" . '$query = db_insert(\'' . $query_table . '\')' . "<br>";
			$act_query .= '->fields(array(';
			foreach ($map_fields_data as $key => $value) {
				 $act_query .= "<br>" . '\'' . $key . '\' => ' . '\'' . $value . '\',';
			}
			$act_query .= "<br>" . '))->execute();';
			$act_query .= "<br>" . '}';
			$act_query .= "<br>" . 'catch (Exception $e) {';
			$act_query .= "<br>" . '$transaction->rollback();';
			$act_query .= "<br>" . 'watchdog_exception(\'Query_Issue\', $e);';
			$act_query .= "<br>" . '}';	
		}
		else {
			$act_query = 'Insert data';
		}
	}
	else {
		$act_query = 'Insufficient data!';
	}
	$header = array(array('data' => "Query"));
	$rows = array();
	$rows['query'] = array($act_query);
	$output = theme('table', array('header' => $header, 'rows' => $rows)); 
	return $output;
}

