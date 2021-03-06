<?php

global $examples, $example_options;

if (!function_exists('t')) {
	function t($str) { return $str; }
}

# ------------------------------------------------------------------------------------ #

$example_names['q1'] = t('How many papers have empty vs. non-empty title?');
$example_jobs['q1'] = 226;
$examples['q1'] = <<<BOA
p: Paper = input;
empty: output sum of int;
non_empty: output sum of int;

# check if title is empty or not
if (p.metadata.title == "")
	empty << 1;
else
	non_empty << 1;
BOA;

# ------------------------------------------------------------------------------------ #

$example_names['q2'] = t('What 100 words appear in the most number of paper abstracts?');
$example_jobs['q2'] = 227;
$examples['q2'] = <<<BOA
# What 100 words appear in the most number of paper abstracts?
p: Paper = input;
o: output top(100) of string weight int;

stopwords: set of string = stop_words();
# store all words used in this paper's abstract
abstractWords: set of string;

foreach (i: int; def(p.abstract[i])) {
	paragraphWords: array of string = splitall(lowercase(p.abstract[i].text), " ");

	foreach (j: int; !contains(stopwords, paragraphWords[j]))
		add(abstractWords, paragraphWords[j]);
}

words := values(abstractWords);
foreach (k: int; def(words[k]))
	o << words[k] weight 1;
BOA;

# ------------------------------------------------------------------------------------ #

$example_names['q3'] = t('What 100 words appear in the most number of paper bodies?');
$example_jobs['q3'] = 228;
$examples['q3'] = <<<BOA
# What 100 words appear in the most number of paper bodies?
p: Paper = input;
o: output top(100) of string weight int;

stopwords: set of string = stop_words();
# store all words used in this paper's body
bodyWords: set of string;

foreach (i: int; def(p.body_text[i])) {
	foreach (m: int; def(p.body_text[i].body[m])) {
		paragraphWords: array of string = splitall(lowercase(p.body_text[i].body[m].text), " ");

		foreach (j: int; !contains(stopwords, paragraphWords[j]))
			add(bodyWords, paragraphWords[j]);
	}
}

words := values(bodyWords);
foreach (k: int; def(words[k]))
	o << words[k] weight 1;
BOA;

# ------------------------------------------------------------------------------------ #

$example_names['q4'] = t('What 500 words appear in the least number of paper bodies for papers with vaccine in the title?');
$example_jobs['q4'] = 229;
$examples['q4'] = <<<BOA
# What 500 words appear in the least number of paper bodies for papers with vaccine in the title?
p: Paper = input;
o: output bottom(500) of string weight int;

stopwords: set of string = stop_words();
# store all words used in this paper's body
bodyWords: set of string;

if (strfind("vaccine", lowercase(p.metadata.title)) >= 0) {
	foreach (i: int; def(p.body_text[i])) {
		foreach (m: int; def(p.body_text[i].body[m])) {
			paragraphWords: array of string = splitall(lowercase(p.body_text[i].body[m].text), " ");

			foreach (j: int; !contains(stopwords, paragraphWords[j]))
				# match only english words
				if (match("^[a-z]+$", paragraphWords[j]))
					add(bodyWords, paragraphWords[j]);
		}
	}

	words := values(bodyWords);
	foreach (k: int; def(words[k]))
		o << words[k] weight 1;
}
BOA;

# ------------------------------------------------------------------------------------ #

$example_names['q5'] = t('What are the most mentioned antivirals?');
$example_jobs['q5'] = 230;
$examples['q5'] = <<<BOA
# What are the most mentioned antivirals?
p: Paper = input;
o: output top(500) of string weight int;

stopwords: set of string = stop_words();
# store all words used in this paper's body
bodyWords: set of string;

foreach (i: int; def(p.body_text[i])) {
	foreach (m: int; def(p.body_text[i].body[m])) {
		paragraphWords: array of string = splitall(lowercase(p.body_text[i].body[m].text), " ");

		foreach (j: int; !contains(stopwords, paragraphWords[j]))
			# match only english words
			if (match("^[a-z]*vir$", paragraphWords[j]))
				add(bodyWords, paragraphWords[j]);
	}
}

words := values(bodyWords);
foreach (k: int; def(words[k]))
	o << words[k] weight 1;
BOA;

# ------------------------------------------------------------------------------------ #

$example_names['q6'] = t('What do we know about cornavirus risk factors?');
$example_jobs['q6'] = 231;
$examples['q6'] = <<<BOA
## What do we know about cornavirus risk factors?
p: Paper = input;

smoke:     output set of string;
pregnant:  output set of string;
neonatal:  output set of string;
pulmonary: output set of string;

foreach (i: int; def(p.abstract[i])) {
	absStore: string = lowercase(p.abstract[i].text);

	if ((strfind("smoke", absStore) >= 0 || strfind("smoking", absStore) >= 0) && strfind("risk", absStore) >= 0) {
		smoke << p.metadata.title;
	}
	if ((strfind("pregnant", absStore) >= 0 || strfind("pregnancy", absStore) >= 0) && strfind("risk", absStore) >= 0) {
		pregnant << p.metadata.title;
	}
	if ((strfind("neonatal", absStore) >= 0 || strfind("neo-natal", absStore) >= 0) && strfind("risk", absStore) >= 0) {
		neonatal << p.metadata.title;
	}
	if (strfind("pulmonary", absStore) >= 0 && strfind("risk", absStore) >= 0) {
		pulmonary << p.metadata.title;
	}
}
BOA;

# ------------------------------------------------------------------------------------ #

$example_names['q7'] = t('What has been published about medical care?');
$example_jobs['q7'] = 232;
$examples['q7'] = <<<BOA
## What has been published about medical care?
p: Paper = input;

nursing:     output set of string;
ards:        output set of string;
ecmo:        output set of string;
ventilation: output set of string;
n95:         output set of string;
regulatory:  output set of string;
medication:  output set of string;

foreach (i: int; def(p.abstract[i])) {
	absStore: string = lowercase(p.abstract[i].text);

	if (strfind("nursing", absStore) >= 0 || strfind("long term care", absStore) >= 0) {
		nursing << p.metadata.title;
	}
	if (strfind("ards", absStore) >= 0 || strfind("acute respiratory distress syndrome", absStore) >= 0) {
		ards << p.metadata.title;
	}
	if (strfind("ecmo", absStore) >= 0 || strfind("extracorporeal membrane oxygenation", absStore) >= 0) {
		ecmo << p.metadata.title;
	}
	if (strfind("ventilation", absStore) >= 0) {
		ventilation << p.metadata.title;
	}
	if (strfind("n95", absStore) >= 0) {
		n95 << p.metadata.title;
	}
	if (strfind("eua", absStore) >= 0 || strfind("clia", absStore) >= 0) {
		regulatory << p.metadata.title;
	}
	if (strfind("medication", absStore) >= 0 && strfind("oral", absStore) >= 0) {
		medication << p.metadata.title;
	}
}
BOA;

# ------------------------------------------------------------------------------------ #


# ------------------------------------------------------------------------------------ #

$example_options = array(
	 0 => t('-- Select Example --'),
	 'q1' => $example_names['q1'],
	 'q2' => $example_names['q2'],
	 'q3' => $example_names['q3'],
	 'q4' => $example_names['q4'],
	 'q5' => $example_names['q5'],
	 'q6' => $example_names['q6'],
	 'q7' => $example_names['q7'],
);

?>
