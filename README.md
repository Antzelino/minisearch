# minisearch
Search words/phrases from documents and sort by relevance

## Compile
`make` or `make all`  
clean with `make clean`

## Run
`./minisearch -i inputDocFile [-k K]`  
A relevant message is displayed if arguments are wrong. Input file of documents should have numbered lines, starting from 0 and incrementing by 1 for each line and some whitespace after the number. Each line is a document. K is the number of most relevant documents to be displayed when searching for a query, the top-K documents (default is 10).

## Commands
* `/help` shows the available commands
* `/clear` clears the screen
* `/search q1 [q2 ... q10]` searches the first 10 words of the query and displays in desceding order score and results
* `/df [word]` document frequency of a word (how many times it's shown) in the documents, or of all the words if no word is given
* `/tf id word` term frequency of a word (how many times it's shown) in a specific document
* `/println id` print a specific document/line
* `/exit` free all data structures and exit the program
