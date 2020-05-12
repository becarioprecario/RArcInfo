\name{get.tablenames}
\alias{get.tablenames}

\title{Function for reading the names of the tables in the coverages}
\description{
This function reads the arc.dat file in the info directory and it returns a list
with some data about the tables. Each element of this list is 
another list with data from a single table.
}


\usage{get.tablenames(infodir)}

\arguments{
\item{infodir}{info dir where there is a file called arc.dat}
}

\value{
A list with some information about the tables stored in all the coverages:

\itemize{

\item{Table Name}

\item{Info File}

\item{Number of fields}

\item{Record Size}

\item{Number of records}

\item{Internal/External Table (FALSE/TRUE)}
	}
}


\keyword{file}
