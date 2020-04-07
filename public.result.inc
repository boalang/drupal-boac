<?php

function boa_public_result($jobId) {
	global $user;

	if (!isValidJob($jobId))
		invalidJob($jobId);

	drupal_set_title(t('Result ') . $jobId);

	$query = db_select('boa_jobs', 'j');
	$query->join('boa_output', 'o', 'j.id = o.id');
	$query->fields('j', array('id', 'uid', 'source', 'deleted', 'submitted',
		'public', 'compiler_status', 'hadoop_status'));
	$query->fields('o', array('count'));
	$query->addExpression('(SELECT name FROM boa_input WHERE id = j.input)', 'name');
	$query->addExpression('COALESCE(o.length, 0)', 'length');
	$query->addExpression('COALESCE(CHAR_LENGTH(o.web_result), 0)', 'weblen');
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

.context .result {
	border: 1px solid #d3d3d3;
	margin-top: 3px;
}

.context .result .header {
	background-color: #d3d3d3;
	padding: 3px;
	cursor: pointer;
	font-weight: bold;
}

.context .result .para {
	display: none;
	padding: 5px;
}

.context .para b {
	color: #e00;
	font-weight: bold;
}
</style>
CSS;

	if ($record->deleted)
		$content .= '<h2>NOTE: This query is deleted.</h2><br/>';
	$content .= '<b>Created By:</b> ' . user_load($record->uid)->name . '<br/>';
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
		if ($record->count > 0)
			$content .= '<b>Papers Found:</b> ' . $record->count . "<br/>";
		else
			$content .= '<b>Result Size:</b> ' . formatBytes($record->length) . "<br/>";
	}

	# -----------------------------------------------

	$content .= '<h2>Query Parameters</h2>';

	$loc = 'search_paper';
	if (strpos($record->source, 'search_title') !== false)
		$loc = 'search_title';
	else if (strpos($record->source, 'search_abstract') !== false)
		$loc = 'search_abstract';
	else if (strpos($record->source, 'search_body') !== false)
		$loc = 'search_body';
	else if (strpos($record->source, 'has_finding') !== false)
		$loc = 'has_finding';

	if (preg_match_all('/if \((?:!filtered && )?!' . $loc . '\("([^"]+)", input\)\)/', $record->source, $matches))
		foreach ($matches[1] as $word)
			if (strpos($word, ' ') === false)
				$content .= '<b>Must contain word</b>: ' . "$word<br/>\n";
			else
				$content .= '<b>Must contain phrase</b>: ' . "$word<br/>\n";
	if (preg_match_all('/if \((?:!filtered && )?' . $loc . '\("([^"]+)", input\)\)/', $record->source, $matches))
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
		case 'has_finding':
			$content .= "in the findings of the article<br/>\n";
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
	$query = db_select('boa_jobs', 'j');
	$query->join('boa_output', 'o', 'j.id = o.id');
	$query->fields('j', array('hadoop_status'));
	$query->fields('o', array('web_result', 'count', 'web_count'));
	$query->addExpression('COALESCE(o.length, 0)', 'length');
	$query->condition('j.id', $jobId);

	$record = $query->execute()->fetchObject();

	if ($record->hadoop_status != 2)
		return "";

	if ($record->length <= 0)
		return "No articles matched the query parameters.\n";

	if ($record->count > 0)
		$trimmed = $record->count > $record->web_count;
	else
		$trimmed = $record->length > $record->weblen;
	$content = "";
	if ($trimmed) {
		if ($record->count > 0)
			$content .= "\nNote: Result is <b>" . $record->count . " papers</b>, only showing first " . $record->web_count . " papers\n";
		else
			$content .= "\nNote: Result is <b>" . formatBytes($record->length) . "</b>, only showing first " . formatBytes($record->weblen) . "\n";
	}
	$rows = explode("\n", substr($record->web_result, 0, strrpos($record->web_result, "\n")));
	$year = -1;
	foreach ($rows as $row) {
		$newyear = date("Y") - (int)substr($row, 2, strpos($row, ']'));
		if ($year != $newyear)
			$content .= "\t<h3>$newyear</h3>\n";
		$year = $newyear;
		$s = linkify(htmlspecialchars($row), array('target' => '_blank'));
		$s = str_replace('[b]', '<b>', $s);
		$s = str_replace('[/b]', '</b>', $s);
		$s = str_replace('[/div]', '</div>', $s);
		$s = str_replace('[div class=&quot;context&quot;]', '<div class="context">', $s);
		$s = str_replace('[div class=&quot;para&quot;]', '<div class="para">', $s);
		$content .= "\t<li>" . substr($s, strpos($s, '] = ') + 4) . "</li>\n";
	}
	if ($trimmed)
		$content .= "... remaining output not shown, please <a href=\"?q=boa/result/public/$jobId/download\">download output</a>.\n";

	$content .= <<<JAVASCRIPT
<script type="text/javascript">
<!--//--><![CDATA[//><!--
(function ($) {
	\$(".context .para").wrap("<div class=\"result\"></div>");
	\$(".context .para").before("<div class=\"header\"><span>&#9660; Expand Result</span></div>");

	\$(".context .header").click(function () {
		\$header = \$(this);
		\$content = \$header.next();

		if (\$content.is(":visible"))
			\$header.html("&#9660; Expand Result");
		else
			\$header.html("&#9650; Collapse Result");

		\$content.slideToggle(300);
	});
}(jQuery));
//--><!]]>
</script>
JAVASCRIPT;

	return $content;
}

?>
