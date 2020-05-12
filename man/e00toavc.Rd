\name{e00toavc}
\alias{e00toavc}

\title{Converts an ESRI E00 file into an Arc/Info V 7.x binary coverage}
\description{
This function makes a convertion from an ESRI E00 file to a binary
coverage. Usually two new directories
are created. One with the name of the coverage and another called 'info',
where some information about the tables are created. If this directory
already exists (because there are already other binary coverages), 
then the new information is added and no file is replaced or deleted.

}

\usage{e00toavc(e00file, avcdir)}

\arguments{
\item{e00file}{The E00 file to be converted.}
\item{avcdir}{The path to the binary coverage directory we want to create.}
}

\value{
Returns 'NULL' on exit.
}

\seealso{avctoe00}


\keyword{file}

