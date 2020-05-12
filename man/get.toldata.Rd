\name{get.toldata}
\alias{get.toldata}

\title{Function for importing the contents of a TOL file into R}
\description{
This function reads and imports into R the  contents of a tolerance definition file.
}

\usage{get.toldata(datadir, coverage, filename="tol.adf")}

\arguments{
\item{datadir}{Directory under which all the coverages and a directory called 'info' are.}
\item{coverage}{The name of the coverage we want to work with}
\item{filename}{The name of the file in the coverage directory that
stores the data. By default it is called 'tol.adf'. In some cases, when
the values are stored in double precission,
the tolerances are in a file whose name is 'par.adf'.} 
}

\value{
This function returns a data frame with the next fields (columns):

\item{Type}{A number from 1 to 10 showing the tolerance type.} 

\item{Status}{This field indicates whether the tolerance is active or not.}

\item{Value}{Tolerance value.}
}


\keyword{file}
