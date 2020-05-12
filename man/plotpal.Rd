\name{plotpal}
\alias{plotpal}

\title{Plots the data imported from an ARC file according to the contents
of a PAL file}

\description{
This function works like plotarc, but we can also decide what polygons to
plot. The arc definitions are stored in the arc variable, and the polygon 
definitions are in the pal variable.
}

\usage{plotpal(arc, pal, new=TRUE, index, ...)}

\arguments{
\item{arc}{The data returned by a call to get.arcdata}
\item{pal}{The data returned by a call to get.paldata}
\item{new}{Do you want to plot on the last window/device or on a new one?}
\item{index}{The indices of the polygons to be plotted.}
\item{...}{Options to be passed to a call to the function plot when
creating the display (i. e., window or file).}
}

\value{
This function returns nothing, but plots a nice map :-D.
}

\keyword{hplot}
