# minisearch
Search words/phrases from documents and sort by relevance

## Compile
`make` or `make all`

## Run
`./minisearch -i inputDocFile [-k K]`
A relevant message is displayed if arguments are wrong. Input file of documents should have numbered lines, starting from 0 and incrementing by 1 for each line and some whitespace after the number. Each line is a document. K is the number of most relevant documents to be displayed when searching for a query, the top-K documents.
