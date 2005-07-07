/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/util/QuickSort.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */

package org.sipfoundry.util;

/**
 * QuickSort.java
 * Used source code from http://www.projector.com/Download/QuickSort.html
 * which according to the info on the website "may be used freely".
 *
 * sorting ints( same algorithm) based on James Golsing's
 * http://www.cs.rit.edu/~atk/Java/Sorting/QSortAlgorithm.java
 *
 * Created: Thu Oct 11 18:13:02 2001
 *
 * @version 1.0
 */

public class QuickSort {

    /**
     * Sorts the array of Comparable objects using the QuickSort algorithm.
     * @param comparable an array of java.lang.Comparable objects
     */
    public void sort(Comparable comparable[]) {
      qSort(comparable, 0, comparable.length - 1);
    }

    /**
     * Sorts the array of ints using the QuickSort algorithm.
     * @param comparable an array of int
     */
    public void sort(int intArray[]) throws Exception {
        qSort(intArray, 0, intArray.length-1);
    }

    /**
     * Sorts the array of Strings using the QuickSort algorithm.
     * @param comparable an array of Strings
     */
    public void sort(String strArray[]) throws Exception {
        qSort(strArray, 0, strArray.length-1);
    }


    /**
     * Sorts the array of objects using the QuickSort algorithm and the
     * supplied comparer.
     *
     * @param objArray The array of objects that should be sorted
     * @param comparer The comparer routine used to compare the objects
     */
    public void sort(Object objArray[], Comparer comparer) throws Exception
    {
        qSort(objArray, comparer, 0, objArray.length-1) ;
    }


   /**
    * Sorts the section of the array passed - the indices of which are between
    * lowInt and highInt
    * @param comparable
    *    an array of java.lang.Comparable(org.sipfoundry.util.Comparable) objects
    * @param lowInt     int containing lowest index of array partition
    * @param highInt    int containing highest index of array partition
    */
   void qSort(Object objects[], Comparer comparer, int lowInt, int highInt) {
      int low  = lowInt;
      int high = highInt;
      Object  middle;

      // The partion to be sorted is split into two separate sections which are
      // sorted individually. This is done by arbitrarily establishing the
      // middle
      // object as the starting point.
      if (highInt > lowInt) {
          middle = objects[(lowInt + highInt) / 2];

         // Increment low and decrement high until they cross.
         while (low <= high) {
            // Increment low until low >= highInt or the comparison between
            // middle and comparable[low] no longer yields -1
             // (ie comparable[low]
            // is >= middle).
            while (low < highInt && (comparer.compare(objects[low], middle) < 0)) {
               ++low;
            }

            // Decrement high until high <= lowInt or the comparison between
            // middle and comparable[high] no longer yields 1
            // (ie comparable[high]
            // is <= middle).
            while (high > lowInt &&
                   (comparer.compare(objects[high], middle) > 0 )) {
               --high;
            }

            // We know have comparable[high] <= middle <= comparable[low] thus
            // the values of comparable[high] and comparable[low] need to be
            // swapped as to yield comparable[low] <= middle <=
            // comparable[high] if low <= high
            if (low <= high) {
               Object obj;
               obj = objects[low];
               objects[low]  = objects[high];
               objects[high] = obj;
               ++low;
               --high;
            }
         }

         // We now have established three sections, viz:
         // if low >= high
         //    comparable[lowInt] to comparble[high]
         //    middle
         //    comparable[low] to comparble[highInt]
         // or if low <= high
         //    comparable[lowInt] to comparble[low]
         //    middle
         //    comparable[high] to comparble[highInt]
         if (lowInt < high) {
            qSort(objects, comparer, lowInt, high);
         }

         if (low < highInt) {
            qSort(objects, comparer, low, highInt);
         }
      }
   }



   /**
    * Sorts the section of the array passed - the indices of which are between
    * lowInt and highInt
    * @param comparable
    *    an array of java.lang.Comparable(org.sipfoundry.util.Comparable) objects
    * @param lowInt     int containing lowest index of array partition
    * @param highInt    int containing highest index of array partition
    */
   void qSort(Comparable comparable[], int lowInt, int highInt) {
      int low  = lowInt;
      int high = highInt;
      Comparable middle;

      // The partion to be sorted is split into two separate sections which are
      // sorted individually. This is done by arbitrarily establishing the
      // middle
      // object as the starting point.
      if (highInt > lowInt) {
          middle = comparable[(lowInt + highInt) / 2];

         // Increment low and decrement high until they cross.
         while (low <= high) {
            // Increment low until low >= highInt or the comparison between
            // middle and comparable[low] no longer yields -1
             // (ie comparable[low]
            // is >= middle).
            while (low < highInt && (comparable[low].compareTo(middle) < 0)) {
               ++low;
            }

            // Decrement high until high <= lowInt or the comparison between
            // middle and comparable[high] no longer yields 1
            // (ie comparable[high]
            // is <= middle).
            while (high > lowInt &&
                   (comparable[high].compareTo(middle) > 0 )) {
               --high;
            }

            // We know have comparable[high] <= middle <= comparable[low] thus
            // the values of comparable[high] and comparable[low] need to be
            // swapped as to yield comparable[low] <= middle <=
            // comparable[high] if low <= high
            if (low <= high) {
               Comparable obj;
               obj = comparable[low];
               comparable[low]  = comparable[high];
               comparable[high] = obj;
               ++low;
               --high;
            }
         }

         // We now have established three sections, viz:
         // if low >= high
         //    comparable[lowInt] to comparble[high]
         //    middle
         //    comparable[low] to comparble[highInt]
         // or if low <= high
         //    comparable[lowInt] to comparble[low]
         //    middle
         //    comparable[high] to comparble[highInt]
         if (lowInt < high) {
            qSort(comparable, lowInt, high);
         }

         if (low < highInt) {
            qSort(comparable, low, highInt);
         }
      }
   }


   /**
    * Sorts the section of the String array passed - the indices of which are between
    * lowInt and highInt
    * @param comparable
    *    an array of String objects
    * @param lowInt     int containing lowest index of array partition
    * @param highInt    int containing highest index of array partition
    */
   private void qSort(String comparable[], int lowInt, int highInt) {
      int low  = lowInt;
      int high = highInt;
      String     middle;

      // The partion to be sorted is split into two separate sections which are
      // sorted individually. This is done by arbitrarily establishing the
      // middle
      // object as the starting point.
      if (highInt > lowInt) {
          middle = comparable[(lowInt + highInt) / 2];

         // Increment low and decrement high until they cross.
         while (low <= high) {
            // Increment low until low >= highInt or the comparison between
            // middle and comparable[low] no longer yields -1
             // (ie comparable[low]
            // is >= middle).
            while (low < highInt && (comparable[low].compareTo(middle) < 0)) {
               ++low;
            }

            // Decrement high until high <= lowInt or the comparison between
            // middle and comparable[high] no longer yields 1
            // (ie comparable[high]
            // is <= middle).
            while (high > lowInt &&
                   (comparable[high].compareTo(middle) > 0 )) {
               --high;
            }

            // We know have comparable[high] <= middle <= comparable[low] thus
            // the values of comparable[high] and comparable[low] need to be
            // swapped as to yield comparable[low] <= middle <=
            // comparable[high] if low <= high
            if (low <= high) {
               String obj;
               obj = comparable[low];
               comparable[low]  = comparable[high];
               comparable[high] = obj;
               ++low;
               --high;
            }
         }

         // We now have established three sections, viz:
         // if low >= high
         //    comparable[lowInt] to comparble[high]
         //    middle
         //    comparable[low] to comparble[highInt]
         // or if low <= high
         //    comparable[lowInt] to comparble[low]
         //    middle
         //    comparable[high] to comparble[highInt]
         if (lowInt < high) {
            qSort(comparable, lowInt, high);
         }

         if (low < highInt) {
            qSort(comparable, low, highInt);
         }
      }
   }



  /**
    * Sorts the section of the array passed - the indices of which are between
    * lo0 and hi0
    * @param a an array of ints
    * @param lo0    int containing lowest index of array partition
    * @param hi0    int containing highest index of array partition
    */
   private void qSort(int a[], int lo0, int hi0) throws Exception {
        int lo = lo0;
        int hi = hi0;
        if (lo >= hi) {
            return;
        }
        int mid = a[(lo + hi) / 2];
        while (lo < hi) {
            while (lo<hi && a[lo] < mid) {
                lo++;
            }
            while (lo<hi && a[hi] > mid) {
                hi--;
            }
            if (lo < hi) {
                int T = a[lo];
                a[lo] = a[hi];
                a[hi] = T;

            }
        }
        if (hi < lo) {
            int T = hi;
            hi = lo;
            lo = T;
        }
        qSort(a, lo0, lo);
        qSort(a, lo == lo0 ? lo+1 : lo, hi0);
    }


}// QuickSort








