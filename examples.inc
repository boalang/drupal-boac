<?php

global $examples, $example_options;

if (!function_exists('t')) {
	function t($str) { return $str; }
}

# ------------------------------------------------------------------------------------ #

$example_names['q1'] = t('The 100 most used words in the abstract of the papers');
$example_jobs['q1'] = 55;
$examples['q1'] = <<<BOA
#The 100 most used words in the abstract in the papers
p: Paper = input;
abstactStore: string;
arrAbstract:array of string;
storeKeywords: map[int] of string;
stopwords:set of string;
l:int=0;
o: output top(100) of string weight int;
stopwords=stop_words();
foreach (i: int; def(p.abstract[i]))
	abstactStore = p.abstract[i].text;
	arrAbstract = splitall(abstactStore," ");
	foreach (j: int; def(arrAbstract[j]))
	    if(!contains(stopwords,arrAbstract[j]))
	        storeKeywords[l]=arrAbstract[j];
	        l=l+1;
foreach (k: int; def(storeKeywords[k]))
	o << storeKeywords[k] weight 1;
BOA;

# ------------------------------------------------------------------------------------ #

$example_options = array(
	0 => t('-- Select Example --'),
	'TODO category' => array(
		'q1' => $example_names['q1'],
	),
);

?>
