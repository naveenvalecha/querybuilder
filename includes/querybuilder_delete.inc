<?php

/**
 * @file
 * querybuilder_delete.inc
 *
 * Demonstrates for dynamic delete query.
 */

// Load querybuilder_common file.
module_load_include('inc', 'querybuilder', 'includes/querybuilder_common');

/**
 * Implements querybuilder_delete
 *   A form for delete query.
 *
 * @param $form
 * @param $form_state
 *
 * @return array
 */
function querybuilder_delete($form, &$form_state) {
  $options_char = array('eq' => 'Is equal to',
    'neq' => 'Is not equal to',
    'sw' => 'Start with',
    'nsw' => 'Does not start with',
    'ew' => 'End with',
    'new' => 'Does not end with',
    'em' => 'Is Empty',
    'nem' => 'Not Empty',
  );

  $options_num = array('eq' => 'equal to',
    'neq' => 'Is not equal to',
    'lt' => 'Less than',
    'elt' => 'Less than or equal to',
    'egt' => 'Greater than or equal to',
    'in' => 'IN',
    'nin' => 'Not IN',
    'em' => 'Is Empty',
    'nem' => 'Not Empty',
  );
	
	// Get list of tables name from database.
	$tbl_list = querybuilder_get_tables();

	// Get value from $form_state['values']
  $sel_qry_table = isset($form_state['values']['query_table']) ? $form_state['values']['query_table'] : '_none';
  $sel_qry_fields = isset($form_state['values']['query_fields']) ? $form_state['values']['query_fields'] : '_all';
  
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
					if ($key == "blob" || $key == "longblob" || $key == "mediumblob") {
						 $form['placeholder'][$value] = array(
							'#type' => 'textfield', 
							'#title' => 'Value for ' . $value,
							'#attributes' => array('disabled' => TRUE),
							'#description' => 'Field Type ' . $key,
						);
					}
					else {

						if (strpos($key,'int') !== false || strpos($key,'float') !== false || strpos($key,'decimal') !== false) {
							$options = $options_num;
							$description = $key;
							$description .= "<br>" . "For 'between' operator value can seperate by '|'";
						}
						else {
							$options = $options_char;
							$description = $key;
						}

						 $map_fields[$value] = $key;
						 $form['placeholder'][$value] = array(
							'#type' => 'textfield', 
							'#title' => 'Value for ' . $value,
							'#description' => 'Field Type ' . $description,
						);

						$form['placeholder'][$value . "_operator"] = array(
							'#type' => 'select', 
							'#title' => 'Operator for ' . $value,
							'#options' => $options,
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
			'#markup' => querybuilder_delete_result($form, $form_state),
			'#prefix' => '<div id="query-output">',
			'#suffix' => '</div>',
		);
	}
  return $form;
}

/**
 * Implements querybuilder_delete_result
 *   Build query from user input.
 *
 * @param $form
 * @param $form_state
 *
 * @return string
 *   HTML string output
 */
function querybuilder_delete_result($form, $form_state) {
  $query_table = $form_state['values']['query_table'];
  $act_query = NULL;
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
			$validate_result = FALSE;
			if (isset($form_state['values']['placeholder'][$key]) && !empty($form_state['values']['placeholder'][$key])) {
				$field_value = check_plain($form_state['values']['placeholder'][$key]);
				$chk_multi_val = array();
				$chk_multi_val = explode('|', $field_value);
				foreach ($chk_multi_val as $kkey => $val) {
					// Validate user-input.
					$validate_result = validate_value($data_type, $length, $val);
          if (!$validate_result) {
						break;
					}
				}
      }
			if (isset($form_state['values']['placeholder'][$key . "_operator"]) && !empty($form_state['values']['placeholder'][$key . "_operator"])) {
				$field_value_operator = $form_state['values']['placeholder'][$key . "_operator"];			
				// Get operator.
				$act_oper = actual_operator($field_value_operator);
				if ($validate_result) {
					$map_fields_data[$key] = $field_value;
					$map_fields_operator[$key] = $act_oper;
				}
      }				
		}
  }
  if (isset($map_fields_data) && !empty($map_fields_data)) {
		$act_query = '$query = db_delete(\'' . $query_table . '\')' . "<br>";
		$act_query .= '->condition(array(';
		foreach ($map_fields_data as $key => $value) {
			if ($map_fields_operator[$key] == "LIKE_E") {
				$act_query .= "<br>" . '\'' . $key . '\', \'' . $value . '% \', \'LIKE\',';
			}
			elseif ($map_fields_operator[$key] == "S_LIKE") {
				$act_query .= "<br>" . '\'' . $key . '\', \'%' . $value . '\', \'LIKE\',';
			}
			elseif ($map_fields_operator[$key] == "NOT S_LIKE") {
				$act_query .= "<br>" . '\'' . $key . '\', \'%' . $value . '\', \'NOT LIKE\',';
			}
			elseif ($map_fields_operator[$key] == "NOT LIKE_E") {
				$act_query .= "<br>" . '\'' . $key . '\', \'' . $value . '% \', \'NOT LIKE\',';
			}
			elseif ($map_fields_operator[$key] == "IN") {
				$tem_val = NULL;
				$tem_val = str_replace('|', ',', $value);
				$act_query .= "<br>" . '\'' . $key . '\', array(' . $tem_val . '), \'IN\',';
			}
			elseif ($map_fields_operator[$key] == "NOT IN") {
				$tem_val = NULL;
				$tem_val = str_replace('|', ',', $value);
				$act_query .= "<br>" . '\'' . $key . '\', array(' . $tem_val . '), \'NOT IN\',';
			}
			else {
			 $act_query .= "<br>" . '\'' . $key . '\', ' . $value . ', ' . '\'' . $map_fields_operator[$key] . '\',';
			}
		}
		$act_query .= "<br>" . '))->execute();';
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

