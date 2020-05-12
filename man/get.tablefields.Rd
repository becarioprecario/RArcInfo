\name{get.tablefields}
\alias{get.tablefields}

\title{Function for reading names of the table fields in the coverages}
\description{
This function returns the names of the fields (and its type) in the table whose names are 
provided by the user. 
}


\usage{get.tablefields(infodir,tablename)}

\arguments{
\item{infodir}{Info directory where there is a file called arc.dat}
\item{tablename}{The name of the table from which we want to get the fields}
}

\value{
This function returns a data frame with the next fields (columns):

\item{FieldName}{The name of the field.}

\item{FieldType}{
This is an integer from 1 to 6 that explain the kind of data:

	\itemize{
		
	\item{1}{Date}

	\item{2}{Character String}

	\item{3}{Integer (stored as a character string)}

	\item{4}{Numeric (stored as a character string)}

	\item{5}{Binary integer}

	\item{6}{Binary float}
		}
	}
}


\keyword{file}
