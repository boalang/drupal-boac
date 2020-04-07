<?php

function _get_datasets() {
	$query = get_input_query();
	$query->addExpression('IF(enabled = 1, name, CONCAT(\'[admin] \', name))', 'name');
	$query->orderBy('id', 'DESC');
	return $query->execute();
}

function boa_query_form($form, &$form_state) {
	global $base_path;

	$form['words'] = array(
		'#type' => 'fieldset',
		'#title' => 'Find articles',
		'#collapsible' => TRUE,
		'#collapsed' => FALSE,
		'allwords' => array(
			'#type' => 'textfield',
			'#title' => t('with <b>all</b> of the words'),
		),
		'phrase' => array(
			'#type' => 'textfield',
			'#title' => t('with the <b>exact phrase</b>'),
		),
		'without' => array(
			'#type' => 'textfield',
			'#title' => t('<b>without</b> the words'),
		),
	);

	$form['loc'] = array(
		'#type' => 'select',
		'#title' => t('Where my words occur'),
		'#options' => array(
			'1' => t('anywhere in the article'),
			'2' => t('in the title of the article'),
			'3' => t('in the abstract of the article'),
			'4' => t('in the body of the article'),
			'5' => t('in the findings of the article'),
		),
	);

	$years = array();
	for ($i = 1956; $i <= date("Y"); $i++)
		$years[$i] = (string)$i;
	unset($years[1957]);
	unset($years[1958]);
	unset($years[1959]);
	unset($years[1961]);
	unset($years[1963]);
	unset($years[1965]);
	unset($years[1967]);

	$form['range'] = array(
		'#type' => 'fieldset',
		'#title' => t('Return articles <b>dated</b> between years'),
		'#collapsible' => TRUE,
		'#collapsed' => TRUE,
		'start' => array(
			'#type' => 'select',
			'#options' => $years,
			'#default_value' => '1956',
		),
		'end' => array(
			'#type' => 'select',
			'#options' => $years,
			'#default_value' => (string)date("Y"),
		),
	);

	/*
	$form['input'] = array(
		'#type' => 'select',
		'#title' => t('Input Dataset'),
		'#default_value' => isset($form['input']['#default_value']) ? $form['input']['#default_value'] : 0
	);

	$result = _get_datasets();
	foreach ($result as $record)
		$form['input']['#options'][$record->id] = $record->name;
		*/

	$form['run'] = array(
		'#type' => 'submit',
		'#value' => t('Query'),
	);

	$form['note'] = array(
		'#markup' => t('<span style="font-size: 0.8em;">NOTE: All data submitted to this site is subject to our <a href="/policies/privacy.php" target="_blank">privacy policy</a>.</span>'),
	);

	if (is_boa_admin())
		$form['debug'] = array(
			'#type' => 'checkbox',
			'#title' => t('Debug Query'),
		);

	return $form;
}

function boa_query_form_submit($form, &$form_state) {
	global $user;

	//$input = $form_state['values']['input'];
	$input = 2;
	if (!is_valid_input($input))
		invalid_input();

	if (empty($form_state['values']['allwords'])
			&& empty($form_state['values']['phrase'])
			&& empty($form_state['values']['without'])) {
		drupal_set_message(t('Query is empty!'), 'error');
		drupal_goto('boa/run');
		drupal_exit();
	}

	$query = '';

	$loc = isset($form_state['values']['loc']) ? (int)$form_state['values']['loc'] : -1;
	switch ($loc) {
		default:
			$searchFunc = "";
			break;
		case 1:
			$searchFunc = "search_paper";
			break;
		case 2:
			$searchFunc = "search_title";
			break;
		case 3:
			$searchFunc = "search_abstract";
			break;
		case 4:
			$searchFunc = "search_body";
			break;
		case 5:
			$searchFunc = "has_finding";
			break;
	}

	if (!empty($form_state['values']['allwords'])) {
		$query .= "if (!filtered) {\n";
		$words = preg_split('/\s+/', trim($form_state['values']['allwords']));
		asort($words);
		foreach ($words as $k)
			$query .= "\tif (!filtered && !$searchFunc(\"$k\", input))\n\t\tfiltered = true;\n";
		$query .= "}\n\n";
	}
	if (!empty($form_state['values']['phrase'])) {
		$query .= "if (!filtered) {\n";
		$query .= "\tif (!filtered && !$searchFunc(\"" . trim($form_state['values']['phrase']) . "\", input))\n\t\tfiltered = true;\n";
		$query .= "}\n\n";
	}
	if (!empty($form_state['values']['without'])) {
		$query .= "if (!filtered) {\n";
		$words = preg_split('/\s+/', trim($form_state['values']['without']));
		asort($words);
		foreach ($words as $k)
			$query .= "\tif (!filtered && $searchFunc(\"$k\", input))\n\t\tfiltered = true;\n";
		$query .= "}\n\n";
	}
	if (!empty($form_state['values']['start'])) {
		$query .= "if (!filtered) {\n";
		$query .= "\tif (yearof(input.metadata.publish_time) < " . $form_state['values']['start'] . ")\n\t\tfiltered = true;\n";
		$query .= "}\n\n";
	}
	if (!empty($form_state['values']['end'])) {
		$query .= "if (!filtered) {\n";
		$query .= "\tif (yearof(input.metadata.publish_time) > " . $form_state['values']['end'] . ")\n\t\tfiltered = true;\n";
		$query .= "}\n\n";
	}

	$source = "o: output collection[string] of string;\n\n";
	$source .= "filtered := !def(input.metadata) || !def(input.metadata.doi_url) || !def(input.metadata.publish_time);\n";
	$source .= "context := \"\";\n\n";
	$source .= $query;
	if ($searchFunc == "has_finding") {
		$source .= "if (!filtered) {\n";
		$source .= "\tcontext = \" [div class=\\\"context\\\"]\";\n\n";

		$source .= "\tvisit(input, visitor {\n";
		$source .= "\t\tbefore p: Paragraph -> {\n";
		$source .= "\t\t\tif (";
		if (!empty($form_state['values']['allwords'])) {
			$words = preg_split('/\s+/', trim($form_state['values']['allwords']));
			asort($words);
			$first = true;
			foreach ($words as $k) {
				if (!$first)
					$source .= " || ";
				$source .= "has_finding(\"$k\", p)";
				$first = false;
			}
		}
		$source .= ")\n";
		$source .= "\t\t\t\tcontext = context + \"[div class=\\\"para\\\"]\" + p.text + \"[/div]\";\n";
		$source .= "\t\t}\n";
		$source .= "\t});\n\n";

		$source .= "\tcontext = context + \"[/div]\";\n";
		if (!empty($form_state['values']['allwords'])) {
			$words = preg_split('/\s+/', trim($form_state['values']['allwords']));
			asort($words);
			foreach ($words as $k)
				$source .= "\tcontext = strreplace(context, \"$k\", \"[b]${k}[/b]\", true);\n";
		}

		$source .= "\n\to[format(\"%03d\", yearof(now()) - yearof(input.metadata.publish_time))] << pretty_print(input) + context;\n";
		$source .= "}";
	} else {
		$source .= "if (!filtered) o[format(\"%03d\", yearof(now()) - yearof(input.metadata.publish_time))] << pretty_print(input) + context;";
	}

	if (isset($form_state['values']['debug']) && (bool)$form_state['values']['debug'] && is_boa_admin()) {
		print("<pre>");
		print($source);
		print("</pre>");
		drupal_exit();
	}

	$running = job_run_count($user->uid);

	$query = db_insert('boa_jobs');
	$tz = date_default_timezone_get();
	date_default_timezone_set('America/Chicago');
	$query->fields(array(
		'submitted' => date('Y-m-d G:i:s'),
		'uid' => $user->uid,
		'parent_id' => NULL,
		'source' => $source,
		'input' => $input,
	));
	date_default_timezone_set($tz);
	$id = $query->execute();

	$query = db_insert('boa_output');
	$query->fields(array(
		'id' => $id,
		'length' => 0,
		'web_result' => ''
	));
	$query->execute();

	drupal_set_message(t('Your job was submitted.  Please note it can take several minutes to get results, depending on server load.'));
	if ($running > 0)
		drupal_set_message(t('You have other job(s) running.  This job will queue until those jobs finish or are stopped.'));
	if (job_run_count() > 3)
		drupal_set_message(t('Server load is currently high.  It will take some time for this job to finish.'));
	drupal_goto("boa/result/$id");
	drupal_exit();
}

?>
