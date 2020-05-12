\name{get.txtdata}
\alias{get.txtdata}


\title{Function for importing the contents of an TXT file into R}
\description{
This function reads and imports into R the  contents of a file of annotations (TXT).
}


\usage{get.txtdata(datadir, coverage, filename="txt.adf")}

\arguments{
\item{datadir}{Directory under which all the coverages and a directory called 'info' are.}
\item{coverage}{The name of the coverage we want to work with.}
\item{filename}{The name of the file in the coverage directory that
stores the data. By default, it is called 'txt.dat'.}
}

\value{
This function returns a list with two elements. The first one is a dataframe
with the next columns:

\item{TxtID}{This field identifies the anotation.}

\item{UserID}{Identifier defined by the user.}

\item{Level}{I don't know what this exactly means. Please, help me to fix this.}

\item{NVerticesLine}{Number of vertices pairs that are valid.}

\item{NVerticesArrow}{I don't know what this exactly means. Please, help me to fix this.}

\item{Text}{Some text related to the annotation.}


The second element in the list is another list containing the vertices
related the annotation. For each annotation there are two vectors, for
the X and Y coordinates.
}



\keyword{file}
