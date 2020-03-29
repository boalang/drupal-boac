<?php

global $examples, $example_options;

if (!function_exists('t')) {
	function t($str) { return $str; }
}

# ------------------------------------------------------------------------------------ #

$example_names['q0'] = t('How many papers have empty vs. non-empty title?');
$example_jobs['q0'] = 145;
$examples['q0'] = <<<BOA
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

$example_names['q1'] = t('What 100 words appear in the most number of paper abstracts?');
$example_jobs['q1'] = 149;
$examples['q1'] = <<<BOA
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

$example_names['q2'] = t('What 100 words appear in the most number of paper bodies?');
$example_jobs['q2'] = 150;
$examples['q2'] = <<<BOA
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

$example_names['q3'] = t('What are the 500 most uncommon words in the papers related to vaccines?');
$example_jobs['q3'] = 96;
$examples['q3'] = <<<BOA
# What are the 500 most uncommon words in the papers related to vaccines?
p: Paper = input;

o: output bottom(500) of string weight int;

bodyStore: string;
arrBody: array of string;
storeKeywords: map[int] of string;
stopwords: set of string = stop_words();
l: int = 0;

if (strfind("vaccine", p.metadata.title) >= 0) {
	foreach (i: int; def(p.body_text[i])) {
		foreach (m: int; def(p.body_text[i].body[m])) {
			bodyStore = p.body_text[i].body[m].text;
			arrBody = splitall(bodyStore, " ");

			foreach (j: int; def(arrBody[j])) {
				if (!contains(stopwords, lowercase(string(arrBody[j])))) {
					if (len(arrBody[j]) > 5) {
						if ((!match("[()0-9$@$!%*?&#^'\"*=~><:;/,\\-_. +\n]+$", lowercase(string(arrBody[j])))) && (match("(.*:*)$", lowercase(string(arrBody[j])))))
						{
							storeKeywords[l] = string(arrBody[j]);
							l = l + 1;
						}
					}
				}
			}
		}
	}
}

foreach (k: int; def(storeKeywords[k]))
	o << storeKeywords[k] weight 1;
BOA;

# ------------------------------------------------------------------------------------ #

$example_names['q4'] = t('What are the most used antivirals?');
$example_jobs['q4'] = 97;
$examples['q4'] = <<<BOA
# What are the most used antivirals?
p: Paper = input;

o: output bottom(500) of string weight int;

bodyStore: string;
arrBody: array of string;
storeKeywords: map[int] of string;
stopwords: set of string = stop_words();
l: int = 0;

foreach (i: int; def(p.body_text[i])) {
	foreach (m: int; def(p.body_text[i].body[m])) {
		bodyStore = p.body_text[i].body[m].text;
		arrBody = splitall(bodyStore, " ");

		foreach (j: int; def(arrBody[j])) {
			if (!contains(stopwords, lowercase(string(arrBody[j])))) {
				if (len(arrBody[j]) > 3) {
					if (!match("^[()0-9$@$!%*?&#^'\"*=~><:;/,\\-_. +\\n]+$", lowercase(string(arrBody[j])))) {
						if (match("(.*vir)$", lowercase(string(arrBody[j])))) {
							storeKeywords[l] = string(arrBody[j]);
							l = l + 1;
						}
					}
				}
			}
		}
	}
}

foreach (k: int; def(storeKeywords[k]))
	o << storeKeywords[k] weight 1;
BOA;

# ------------------------------------------------------------------------------------ #

$example_names['q5'] = t('What do we know about COVID-19 risk factors?');
$example_jobs['q5'] = 112;
$examples['q5'] = <<<BOA
# What do we know about COVID-19 risk factors?
p: Paper = input;

smoke: output collection of string;
pregnant: output collection of string;
neonatal: output collection of string;
pulmonary: output collection of string;

absStore: string;

foreach (i: int; def(p.abstract[i])) {
	absStore = p.abstract[i].text;
	if (((strfind("smoke", lowercase(absStore)) >= 0)||(strfind("smoking", lowercase(absStore)) >= 0)) && (strfind("risk", lowercase(absStore)) >= 0)) {
		smoke << p.metadata.title;
	}
	if (((strfind("pregnant", lowercase(absStore)) >= 0)||(strfind("pregnancy", lowercase(absStore)) >= 0)) && (strfind("risk", lowercase(absStore)) >= 0)) {
		pregnant << p.metadata.title;
	}
	if (((strfind("neonatal", lowercase(absStore)) >= 0)||(strfind("neo-natal", lowercase(absStore)) >= 0)) && (strfind("risk", lowercase(absStore)) >= 0)) {
		neonatal << p.metadata.title;
	}
	if ((strfind("pulmonary", lowercase(absStore)) >= 0) && (strfind("risk", lowercase(absStore)) >= 0)) {
		pulmonary << p.metadata.title;
	}
}
BOA;

# ------------------------------------------------------------------------------------ #

$example_names['q6'] = t('What has been published about medical care?');
$example_jobs['q6'] = 113;
$examples['q6'] = <<<BOA
# What has been published about medical care?
p: Paper = input;

nursing: output collection of string;
ards: output collection of string;
ecmo: output collection of string;
ventilation: output collection of string;
n95: output collection of string;
regulatory: output collection of string;
medication: output collection of string;

absStore: string;

foreach (i: int; def(p.abstract[i])) {
	absStore = p.abstract[i].text;
	if ((strfind("nursing", lowercase(absStore)) >= 0)||(strfind("long term care", lowercase(absStore)) >= 0)) {
		nursing << p.metadata.title;
	}
	if ((strfind("ards", lowercase(absStore)) >= 0)||(strfind("acute respiratory distress syndrome", lowercase(absStore)) >= 0)) {
		ards << p.metadata.title;
	}
	if ((strfind("ecmo", lowercase(absStore)) >= 0)||(strfind("extracorporeal membrane oxygenation", lowercase(absStore)) >= 0)) {
		ecmo << p.metadata.title;
	}
	if ((strfind("ventilation", lowercase(absStore)) >= 0)) {
		ventilation << p.metadata.title;
	}
	if ((strfind("n95", lowercase(absStore)) >= 0)) {
		n95 << p.metadata.title;
	}
	if ((strfind("eua", lowercase(absStore)) >= 0)||(strfind("clia", lowercase(absStore)) >= 0)) {
		regulatory << p.metadata.title;
	}
	if ((strfind("medication", lowercase(absStore)) >= 0) && (strfind("oral", lowercase(absStore)) >= 0)) {
		medication << p.metadata.title;
	}
}
BOA;

# ------------------------------------------------------------------------------------ #


# ------------------------------------------------------------------------------------ #

$example_options = array(
	 0 => t('-- Select Example --'),
	 'q0' => $example_names['q0'],
	 'q1' => $example_names['q1'],
	 'q2' => $example_names['q2'],
	 'q3' => $example_names['q3'],
	 'q4' => $example_names['q4'],
	 'q5' => $example_names['q5'],
	 'q6' => $example_names['q6'],
);

?>
