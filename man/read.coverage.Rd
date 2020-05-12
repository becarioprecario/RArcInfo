\name{read.coverage}
\alias{read.coverage}

\title{Function for retrieving basic data from a given coverage}
\description{
This function allows the user to retrieve the main data stored in the chosen
coverage. This doesn't mean that all the data are retrieved, but only the
most important.

Tables are not imported, but a dataframe containig the information provided by
a call to get.tablenames of all the tables related to the coverage.

For a single coverage there can be more than one pal file, so just their names
are imported (into 'palfiles'). Then, the user can choose which one to import
by calling get.paldata.  But, if there is just one pal file, it is imported
into 'pal'.

The names of the different files are supposed to be the default names when
calling the proper get.XXXdata. If some of these names change then the
function will not import that feature properly. But this fact should never
happen.
}

\usage{read.coverage(datadir, coverage)}

\arguments{
\item{datadir}{Directory under which all the coverages and a directory called 'info' are.}
\item{coverage}{The name of the coverage we want to work with}
}

\value{
This function returns a list of all the data directory, coverage name
and all the data imported:
i
\item{datadir}{The directory where all the coverages are stored, the one used 
when this function was called.}

\item{coverage}{The name of the coverage used when calling this function.}

\item{arc}{The arc definitions, as returned by get.arcdata.}

\item{bnd}{The bounday definition, as returned by get.bnddata.}

\item{cnt}{The polygon centroids, as returned by get.cntdata.}

\item{lab}{The label point records of the coverage.}

\item{pal}{If there is just one file for the polygon definitions inside the 
coverage, it is imported here using get.paldata. Otherwise, it is filled 
with a 'NULL'.}

\item{palfiles}{The names of all the polygon definitions files.}

\item{tblnames}{The description of all the tables related to the coverage,
as returned by get.tablenames.}

\item{tol}{The tolerance values.}

}

\references{
More information about this kind of data can be found at
\url{http://pages.infinit.net/danmo/e00/docs/v7_bin_cover.html}.
}

\keyword{file}
