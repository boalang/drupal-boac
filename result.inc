<?php

function boa_result($jobId) {
	global $user, $base_path;

	if (!isValidJob($jobId))
		invalidJob($jobId);

	drupal_set_title(t('Query ') . $jobId);

	$query = db_select('boa_jobs', 'j');
	$query->join('boa_output', 'o', 'j.id = o.id');
	$query->fields('j', array('id', 'uid', 'source', 'deleted', 'public',
		'compiler_output', 'compiler_status', 'hadoop_status'));
	$query->addExpression('(SELECT name FROM boa_input WHERE id = j.input)', 'name');
	$query->addExpression('COALESCE(o.length, 0)', 'length');
	$query->addExpression('unix_timestamp(created)', 'created');
	$query->addExpression('unix_timestamp(submitted)', 'submitted');
	$query->addExpression('unix_timestamp(compile_end)', 'compile_end');
	$query->addExpression('unix_timestamp(hadoop_end)', 'hadoop_end');
	$query->addExpression('unix_timestamp(hadoop_end) - unix_timestamp(submitted)', 'finished');
	if (!is_boa_admin())
		$query->condition('deleted', false);
	$query->condition('j.id', $jobId);
	$result = $query->execute();

	$record = $result->fetchObject();

	# -----------------------------------------------

	$content = '';

	$content .= <<<CSS
<style type="text/css" media="all">
.resultlist {
}
</style>
CSS;

	if ($record->compiler_status == 0 || $record->compiler_status == 1 ||
			($record->compiler_status == 2 && ($record->hadoop_status == 0 || $record->hadoop_status == 1)))
		$content .= '<b>Query is currently running</b> <div class="ajax-progress ajax-progress-throbber"><div class="throbber">&nbsp;</div></div><br/>(note: this page automatically reloads)<br/><br/>';

	if ($record->deleted)
		$content .= '<h2>NOTE: This query is deleted.</h2><br/>';
	if ($user->uid != $record->uid)
		$content .= '<b>Created By:</b> <a href="?q=user/' . $record->uid . '">' . user_load($record->uid)->name . '</a><br/>';
	$content .= '<b>Created At:</b> ' . boa_date($record->created) . '<br/>';
	$content .= '<br/>';
	$content .= '<b>Input Dataset:</b> ' . $record->name . '<br/>';
	$content .= '<br/>';
	if ($record->compiler_status == 0 || $record->compiler_status == 1 ||
			($record->compiler_status == 2 && ($record->hadoop_status == 0 || $record->hadoop_status == 1)))
		$content .= '<b>Submitted At:</b> ' . boa_date($record->submitted) . ' (' . boa_duration(time() - $record->submitted) .')<br/>';
	else
		$content .= '<b>Submitted At:</b> ' . boa_date($record->submitted) . '<br/>';
	if ($record->hadoop_status == 2) {
		$content .= '<b>Finished At:</b> ' . boa_date($record->hadoop_end) . " (" . boa_duration($record->finished) . ")<br/>";
		$content .= '<b>Result Size:</b> ' . formatBytes($record->length) . "<br/>";
	}

	# -----------------------------------------------

	$form = drupal_get_form('update_form', $jobId);
	$content .= drupal_render($form);

	if ($record->compiler_status == 0 || $record->compiler_status == 1
			|| ($record->compiler_status == 2 && ($record->hadoop_status == 0 || $record->hadoop_status == 1))) {
		$wait = 22000;
		if ($record->compiler_status == 1 && $record->compile_end > time())
			$wait = ($record->compile_end - time()) * 1000 + 1000;
		else if ($record->hadoop_status == 1 && $record->hadoop_end > time())
			$wait = ($record->hadoop_end - time()) * 1000 + 1000;
		$content .= <<<JAVASCRIPT
<script type="text/javascript">
<!--//--><![CDATA[//><!--
var jobTimer = setInterval(function (){
	location.reload();
}, $wait);
//--><!]]>
</script>
JAVASCRIPT;
	}

	return $content;
}

function update_form($form, &$form_state, $jobId) {
	$form['run'] = array(
		'#type' => 'submit',
		'#value' => "Refresh Job $jobId",
		'#ajax' => array(
			'callback' => 'update_form_callback',
			'wrapper' => 'update_box',
			'progress' => array(
				'type' => 'progress'
			),
			'effect' => 'fade',
		),
		'#prefix' => '<div style="display: none;">',
		'#suffix' => '</div>',
	);

	$form['content'] = array(
		'#markup' => generate_content($jobId),
	);

	$form['#prefix'] = '<div id="update_box">';
	$form['#suffix'] = '</div>';

	return $form;
}

function update_form_callback($form, $form_state) {
	$form['content']['#markup'] = generate_content($form);
	return $form;
}

function generate_content($jobId) {
	global $user;

	if (is_array($jobId))
		$jobId = substr(strrchr($jobId['#action'], '/'), 1);

	if (!isValidJob($jobId))
		return t('Problem loading job status.');

	$query = db_select('boa_jobs', 'r');
	$query->fields('r', array('id', 'uid', 'source', 'submitted',
		'compiler_status', 'compiler_output',
		'hadoop_status', 'hadoop_output'));
	$query->addExpression('unix_timestamp(compile_start)', 'compile_start');
	$query->addExpression('unix_timestamp(compile_end)', 'compile_end');
	$query->addExpression('unix_timestamp(hadoop_start)', 'hadoop_start');
	$query->addExpression('unix_timestamp(hadoop_end)', 'hadoop_end');
	$query->addExpression('unix_timestamp(hadoop_end) - unix_timestamp(hadoop_start)', 'hadoop_span');
	$query->addExpression('unix_timestamp(compile_end) - unix_timestamp(compile_start)', 'compile_span');
	$query->condition('id', $jobId);
	$result = $query->execute();

	$record = $result->fetchObject();

	$content = '';

	# -----------------------------------------------

	$content .= '<h2>Query Parameters</h2>';

	$loc = 'search_paper';
	if (strpos($record->source, 'search_title') !== false)
		$loc = 'search_title';
	else if (strpos($record->source, 'search_abstract') !== false)
		$loc = 'search_abstract';
	else if (strpos($record->source, 'search_body') !== false)
		$loc = 'search_body';

	if (preg_match_all('/if \(!' . $loc . '\("([^"]+)", input\)\)/', $record->source, $matches))
		foreach ($matches[1] as $word)
			if (strpos($word, ' ') === false)
				$content .= '<b>Must contain word</b>: ' . "$word<br/>\n";
			else
				$content .= '<b>Must contain phrase</b>: ' . "$word<br/>\n";
	if (preg_match_all('/if \(' . $loc . '\("([^"]+)", input\)\)/', $record->source, $matches))
		foreach ($matches[1] as $word)
			$content .= '<b>Must NOT contain word</b>: ' . "$word<br/>\n";

	$content .= '<b>Words must appear</b>: ';
	switch ($loc) {
		case 'search_paper':
			$content .= "anywhere in the article<br/>\n";
			break;
		case 'search_title':
			$content .= "in the title of the article<br/>\n";
			break;
		case 'search_abstract':
			$content .= "in the abstract of the article<br/>\n";
			break;
		case 'search_body':
			$content .= "in the body of the article<br/>\n";
			break;
	}
	$content .= "<br/>\n";
	if (preg_match('/if \(yearof\(input\.metadata\.publish_time\) < (\d+)\)/', $record->source, $matches))
		$content .= '<b>Papers starting in year</b>: ' . $matches[1] . "<br/>\n";
	if (preg_match('/if \(yearof\(input\.metadata\.publish_time\) > (\d+)\)/', $record->source, $matches))
		$content .= '<b>Papers ending in year</b>: ' . $matches[1] . "<br/>\n";

	# -----------------------------------------------

	if ($record->hadoop_status == 2) {
		$content .= '<h2>Result</h2>';
		$content .= "<ul class=\"resultlist\">\n" . get_output($jobId) . "</ul>\n";
	}

	# -----------------------------------------------

	return $content;
}

function get_output($jobId) {
	$query = db_select('boa_jobs', 'r');
	$query->join('boa_output', 'o', 'r.id = o.id');
	$query->fields('r', array('hadoop_status'));
	$query->fields('o', array('web_result'));
	$query->addExpression('COALESCE(o.length, 0)', 'length');
	$query->condition('r.id', $jobId);

	$record = $query->execute()->fetchObject();

	if ($record->hadoop_status == 2) {
		if ($record->length > 0) {
			$trimmed = $record->length > strlen($record->web_result);
			$content = "";
			if ($trimmed)
				$content .= "\nNote: Result is <b>" . formatBytes($record->length) . "</b>, only showing first 64k\n";
			$rows = explode("\n", substr($record->web_result, 0, strrpos($record->web_result, "\n")));
			foreach ($rows as $row) {
				$s = linkify(htmlspecialchars($row), array('target' => '_blank'));
				$content .= "\t<li>" . substr($s, strpos($s, '] = ') + 4) . "</li>\n";
			}
			if ($trimmed)
				$content .= "... remaining output not shown, please <a href=\"?q=boa/job/$jobId/download\">download output</a>.\n";

			return $content;
		}
		return "No articles matched the query parameters.\n";
	}

	return "";
}

?>
