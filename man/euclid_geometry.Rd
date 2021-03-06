% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/geometry.R
\name{euclid_geometry}
\alias{euclid_geometry}
\alias{is_geometry}
\alias{cardinality}
\alias{definition_names}
\title{Vector of geometries}
\usage{
is_geometry(x)

cardinality(x)

definition_names(x)
}
\arguments{
\item{x}{An \code{euclid_geometry} object}
}
\description{
The base vector type for geometries in euclid is the \code{euclid_geometry} class.
While this is a virtual class (you cannot construct it, only its subclasses),
many of the common vector operations are defined here. Further it is possible
to check whether a vector is a geometry of any type using \code{is_geometry()}.
While most geometries are atomic, a few are defined by multiples of the same
geometry, e.g. segments and triangles which are defined by 2 and 3 points
respectively. The cardinality of each geometry in a vector can be queried
with \code{cardinality()}. The matrix conversion will place each sub-geometry of
a geometry on a new row, meaning that the final number of rows in a matrix
constructed from \code{x} is not \code{length(x)} but \code{sum(cardinality(x))}.
}
\section{Vector behaviour}{

Geometry vectors in euclid are made to behave as closely as possible to what
you expect from normal R vectors. However, they are implemented as external
pointers to the exact C representation meaning that they cannot be restored
across sessions or saved to RData/RDS files. Despite being external pointers
they mimick R's copy-on-modify semantics so you should not worry about side
effects when changing a geometry vector. Further, since \link{exact_numeric}
vectors only supports finite values, geometry vectors do not have an \code{NA}
value.

The following is a list of standard R methods defined for geometry
vectors:
\itemize{
\item \code{\link[=as.matrix]{as.matrix()}} - converts the geometry to a standard R matrix of numerics
\item \code{\link[=as.character]{as.character()}} - provides a textual representation of the geometry
\item \code{\link[=format]{format()}} - as above
\item \code{\link[=as.list]{as.list()}} - splits the vector into single elements in a list
\item \code{\link[=str]{str()}} - provides a condensed view of the vector
\item \code{\link[=length]{length()}} - gives the number of geometries in the vector
\item \code{\link[=rep]{rep()}} - replicates elements in the vector
\item \code{\link[=dim]{dim()}} - gives the dimensionality of the geometry (2 or 3)
\item [\code{[}] and [\code{[[}] - extract elements from the vector
\item [\verb{[<-}] and [\verb{[[<-}] - assigns elements into the vector. Since \code{NA} values
are not supported it is not possible to assign past the length of the
vector
\item \code{\link{$}} and \code{\link{$<-}} - will throw an error since geometry vectors are unnamed
\item \code{\link[=c]{c()}} - combine multiple geometries of the same type
\item \code{\link[=unique]{unique()}} - returns the unique elements of the vector keeping the order
\item \code{\link[=duplicated]{duplicated()}} - gives whether an element has been seen before in the
vector
\item \code{\link[=anyDuplicated]{anyDuplicated()}} - Tells if any element in the vector is a duplicate
\item \code{\link{==}} and \code{\link{!=}} - Test for equality between elements.
\item \code{\link[=transform]{transform()}} - Transform geometries in the vector according to an
\link{affine_transformation}
}

Do note that since geometries with exact representation is not easily
hashable, the implementation of \code{unique()} and \code{duplicated()} is not very
efficient.
}

