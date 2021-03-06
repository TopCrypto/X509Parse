#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include "x509.h"
#include "asn1.h"
#include "huge.h"
#include "digest.h"
#include "md5.h"
#include "sha.h"
#include "hex.h"

/**
 * Validate that the given ASN.1 node is of the expected tag type and has (at least)
 * the given number of child nodes.  Return true if it passes all checks, false
 * otherwise.
 * This isn't shown in the book.
 */
int validate_node( struct asn1struct *source,   //检查节点信息
                   int expected_tag, 
                   int expected_children,
                   const char *desc )
{
  struct asn1struct *child;
  int counted_children = 0;

  if ( !source )
  {
    fprintf( stderr, "Error - '%s' missing.\n", desc );
    return 0;
  }

  if ( source->tag != expected_tag )
  {
    fprintf( stderr, "Error parsing '%s'; expected a %d tag, got a %d.\n",
      desc, expected_tag, source->tag );
    return 0;
  }

  child = source->children;

  while ( counted_children < expected_children )
  {
    if ( !child )
    {
      fprintf( stderr, "Error parsing '%s'; expected %d children, found %d.\n",
        desc, expected_children, counted_children );
      return 0;
    }
    counted_children++;
    child = child->next;
  }

  return 1;
}

void init_x509_certificate( signed_x509_certificate *certificate )
{
  set_huge( &certificate->tbsCertificate.serialNumber, 1 );
  memset( &certificate->tbsCertificate.issuer, 0, sizeof( name ) ); //虽未分配空间，但此时将其指针赋值为NULL = 0
  memset( &certificate->tbsCertificate.subject, 0, sizeof( name ) );

  certificate->tbsCertificate.subjectPublicKeyInfo.rsa_public_key.modulus =(huge *)  //为第一个指针*huge modulus赋值
    malloc( sizeof( huge ) );

  certificate->tbsCertificate.subjectPublicKeyInfo.rsa_public_key.exponent =(huge *)
    malloc( sizeof( huge ) );

  set_huge( 
    certificate->tbsCertificate.subjectPublicKeyInfo.rsa_public_key.modulus,   //为huge里面的指针rep赋值
    0 );
  set_huge( 
    certificate->tbsCertificate.subjectPublicKeyInfo.rsa_public_key.exponent,
    0 );
  set_huge( &certificate->rsa_signature_value, 0 );
  set_huge( &certificate->dsa_signature_value.r, 0 );
  set_huge( &certificate->dsa_signature_value.s, 0 );
  certificate->tbsCertificate.certificate_authority = 0;
}


void init_x509_msg(x509Info * x509_msg)
{
   memset(x509_msg, 0, sizeof(x509Info));
   memset(&(x509_msg->ds), 0, sizeof( dsa_info ) ); //虽未分配空间，但此时将其指针赋值为NULL = 0
   memset(&(x509_msg->rs), 0, sizeof( ras_info));
}


void free_x509_msg(x509Info * x509_msg)
{
	if(x509_msg->version){ free(x509_msg->version); }
	int t = strlen(x509_msg->serialnumber);
	if(x509_msg->serialnumber) { free(x509_msg->serialnumber); }
	if(x509_msg->issuer){ free(x509_msg->issuer); } 
	t = strlen(x509_msg->issuer);
	if(x509_msg->subject){ free(x509_msg->subject); } 
	t = strlen(x509_msg->subject);
	if(x509_msg->notbefore) { free(x509_msg->notbefore); }
	t = strlen(x509_msg->notbefore);
	if(x509_msg->notafter) { free(x509_msg->notafter); }
	t = strlen(x509_msg->notafter);
	if(x509_msg->algFlag) { free(x509_msg->algFlag); }
	t = strlen(x509_msg->algFlag);

	if(x509_msg->ds.y) {free(x509_msg->ds.y);}  
	if(x509_msg->ds.p) {free(x509_msg->ds.p);} 
	if(x509_msg->ds.q) {free(x509_msg->ds.q);} 
	if(x509_msg->ds.g) {free(x509_msg->ds.g);} 
	if(x509_msg->ds.r) {free(x509_msg->ds.r);} 
	if(x509_msg->ds.s) {free(x509_msg->ds.s);}

	if(x509_msg->rs.exponent) {free(x509_msg->rs.exponent);} 
	if(x509_msg->rs.modulus) {free(x509_msg->rs.modulus);} 
	if(x509_msg->rs.signValue){free(x509_msg->rs.signValue);} 

	if(x509_msg->signAlgorithm) { free(x509_msg->signAlgorithm); }
	if(x509_msg->caflag ){ free(x509_msg->caflag); }
}




static void free_x500_name( name *x500_name )
{
  if ( x500_name->idAtCountryName ) { free( x500_name->idAtCountryName ); }
  if ( x500_name->idAtStateOrProvinceName ) { free( x500_name->idAtStateOrProvinceName ); }
  if ( x500_name->idAtLocalityName ) { free( x500_name->idAtLocalityName ); }
  if ( x500_name->idAtOrganizationName ) { free( x500_name->idAtOrganizationName ); }
  if ( x500_name->idAtOrganizationalUnitName ) { free( x500_name->idAtOrganizationalUnitName ); }
  if ( x500_name->idAtCommonName ) { free( x500_name->idAtCommonName ); }
}

void free_x509_certificate( signed_x509_certificate *certificate )
{
  free_huge( &certificate->tbsCertificate.serialNumber );
  free_x500_name( &certificate->tbsCertificate.issuer );
  free_x500_name( &certificate->tbsCertificate.subject );
  free_huge( 
   certificate->tbsCertificate.subjectPublicKeyInfo.rsa_public_key.modulus );
  free_huge( 
   certificate->tbsCertificate.subjectPublicKeyInfo.rsa_public_key.exponent );
  free( 
   certificate->tbsCertificate.subjectPublicKeyInfo.rsa_public_key.modulus );
  free( 
   certificate->tbsCertificate.subjectPublicKeyInfo.rsa_public_key.exponent );
  free_huge( &certificate->rsa_signature_value );
  free_huge( &certificate->dsa_signature_value.r );
  free_huge( &certificate->dsa_signature_value.s );
}

static int parse_huge( huge *target, struct asn1struct *source )
{
  target->sign = 0;
  target->size = source->length;
  target->rep = ( unsigned char * ) malloc( target->size );
  memcpy( target->rep, source->data, target->size );

  return 0;
}

static const unsigned char OID_md5WithRSA[] = 
  { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x04 };
static const unsigned char OID_sha1WithRSA[] = 
  { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x05 };
static const unsigned char OID_sha1WithDSA[] = 
  { 0x2A, 0x86, 0x48, 0xCE, 0x38, 0x04, 0x03 };


static int parse_algorithm_identifier( signatureAlgorithmIdentifier *target, 
                    struct asn1struct *source )
{
  struct asn1struct *oid = ( struct asn1struct * ) source->children;

  if ( !validate_node( oid, ASN1_OBJECT_IDENTIFIER, 0, "algorithm identifier oid" ) )  // 检查节点信息
  {
    return 2;
  }

  if ( !memcmp( oid->data, OID_md5WithRSA, oid->length ) )
  {
    *target = md5WithRSAEncryption;
  }
  else if ( !memcmp( oid->data, OID_sha1WithDSA, oid->length ) )
  {
    *target = shaWithDSA;
  }
  else if ( !memcmp( oid->data, OID_sha1WithRSA, oid->length ) )
  {
    *target = shaWithRSAEncryption;
  } 
  else
  {
    int i;
    fprintf( stderr, "Unsupported or unrecognized algorithm identifier OID " );
    for ( i = 0; i < oid->length; i++ )
    {
      fprintf( stderr, "%.02x ", oid->data[ i ] );
    }
    fprintf( stderr, "\n" );
    return 2;
  }
 
  return 0;
}

static unsigned char OID_idAtCommonName[] = { 0x55, 0x04, 0x03 };
static unsigned char OID_idAtCountryName[] = { 0x55, 0x04, 0x06 };
static unsigned char OID_idAtLocalityName[] = { 0x55, 0x04, 0x07 };
static unsigned char OID_idAtStateOrProvinceName[] = { 0x55, 0x04, 0x08 };
static unsigned char OID_idAtOrganizationName[] = { 0x55, 0x04, 0x0A }; 
static unsigned char OID_idAtOrganizationalUnitName[] = { 0x55, 0x04, 0x0B };

/**
 * Name parsing is a bit different. Loop through all of the
 * children of the source, each of which is going to be a struct containing
 * an OID and a value. If the OID is recognized, copy it's contents
 * to the correct spot in "target". Otherwise, ignore it.
 */
int parse_name( name *target, struct asn1struct *source )
{
  struct asn1struct *typeValuePair;
  struct asn1struct *typeValuePairSequence;
  struct asn1struct *type;
  struct asn1struct *value;

  target->idAtCountryName = NULL;
  target->idAtStateOrProvinceName = NULL;
  target->idAtLocalityName = NULL;
  target->idAtOrganizationName = NULL;
  target->idAtOrganizationalUnitName = NULL;
  target->idAtCommonName = NULL;

  if ( !validate_node( source, ASN1_SEQUENCE, 1, "name" ) )
  {
    return 1;
  }
 
  typeValuePair = source->children;
  while ( typeValuePair )
  {
    if ( !validate_node( typeValuePair, ASN1_SET, 1, "tag value pair in name" ) )
    {
      return 1;
    }

    typeValuePairSequence = ( struct asn1struct * ) typeValuePair->children;

    if ( !validate_node( typeValuePairSequence, ASN1_SEQUENCE, 2, "tag value pair in name" ) )
    {
      return 2;
    }

    type = ( struct asn1struct * ) typeValuePairSequence->children;

    if ( !validate_node( type, ASN1_OBJECT_IDENTIFIER, 0, "tag value pair in name type" ) )
    {
      return 3;
    }

    value = ( struct asn1struct * ) type->next;

    if ( !( value->tag == ASN1_PRINTABLE_STRING ||   //当其所有的都不满足时，错误
            value->tag == ASN1_TELETEX_STRING ||
            value->tag == ASN1_IA5_STRING ||
            value->tag == ASN1_UTF8_STRING ) )
    {
      fprintf( stderr, "Error parsing tag value pair in name, expected a string tag, got a %d\n",
        value->tag );
      return 4;
    }


    // 解析OID码的分析
    if ( !memcmp( type->data, OID_idAtCountryName, type->length ) )
    {
      target->idAtCountryName = ( char * ) malloc( value->length + 1 );
      memcpy( target->idAtCountryName, value->data, value->length ); 
      target->idAtCountryName[ value->length ] = 0; // 最后一字节补0
    }
    else if ( !memcmp( type->data, OID_idAtStateOrProvinceName, type->length ) )
    {
      target->idAtStateOrProvinceName = ( char * ) malloc( value->length + 1 );
      memcpy( target->idAtStateOrProvinceName, value->data, value->length ); 
      target->idAtStateOrProvinceName[ value->length ] = 0; 
    }
    else if ( !memcmp( type->data, OID_idAtLocalityName, type->length ) )
    {
      target->idAtLocalityName = ( char * ) malloc( value->length + 1 );
      memcpy( target->idAtLocalityName, value->data, value->length ); 
      target->idAtLocalityName[ value->length ] = 0; 
    }
    else if ( !memcmp( type->data, OID_idAtOrganizationName, type->length ) )
    {
      target->idAtOrganizationName = ( char * ) malloc( value->length + 1 );
      memcpy( target->idAtOrganizationName, value->data, value->length ); 
      target->idAtOrganizationName[ value->length ] = 0; 
    }
    else if ( !memcmp( type->data, OID_idAtOrganizationalUnitName, 
              type->length ) ) 
    {
      target->idAtOrganizationalUnitName = ( char * ) 
        malloc( value->length + 1 );
      memcpy( target->idAtOrganizationalUnitName, value->data, value->length );
      target->idAtOrganizationalUnitName[ value->length ] = 0; 
    }
    else if ( !memcmp( type->data, OID_idAtCommonName, type->length ) )
    {
      target->idAtCommonName = ( char * ) malloc( value->length + 1 );
      memcpy( target->idAtCommonName, value->data, value->length ); 
      target->idAtCommonName[ value->length ] = 0; 
    }
    else
    {
     int i;

     // This is just advisory - NOT a problem
     printf( "Skipping unrecognized or unsupported name token OID of " );
     for ( i = 0; i < type->length; i++ )
     {
       printf( "%.02x ", type->data[ i ] );
     }
     printf( "\n" );
   }

   typeValuePair = typeValuePair->next;
  }

  return 0;
} 

static int parse_validity( validity_period *target, struct asn1struct *source )  //解析合法性
{
  struct asn1struct *not_before;
  struct asn1struct *not_after;
  struct tm not_before_tm;
  struct tm not_after_tm;

  if ( !validate_node( source, ASN1_SEQUENCE, 2, "validity" ) )
  {
    return 1;
  }
 
  not_before = source->children;

  if ( ( not_before->tag != ASN1_UTC_TIME ) && ( not_before->tag != ASN1_GENERALIZED_TIME ) )
  {
    fprintf( stderr, "Error parsing not before; expected a date but got a %d\n",
      not_before->tag );
    return 3;
  }
 
  not_after = not_before->next;

  if ( ( not_after->tag != ASN1_UTC_TIME ) && ( not_after->tag != ASN1_GENERALIZED_TIME ) )
  {
    fprintf( stderr, "Error parsing not after; expected a date but got a %d\n",
      not_after->tag );
    return 5;
  }

  // Convert time instances into time_t
  if ( sscanf( ( char * ) not_before->data, "%2d%2d%2d%2d%2d%2d",
       &not_before_tm.tm_year, &not_before_tm.tm_mon, &not_before_tm.tm_mday,
       &not_before_tm.tm_hour, &not_before_tm.tm_min, &not_before_tm.tm_sec ) < 6 )
  {
    fprintf( stderr, "Error parsing not before; malformed date." );
    return 6;
  }
  if ( sscanf( ( char * ) not_after->data, "%2d%2d%2d%2d%2d%2d",
       &not_after_tm.tm_year, &not_after_tm.tm_mon, &not_after_tm.tm_mday,
       &not_after_tm.tm_hour, &not_after_tm.tm_min, &not_after_tm.tm_sec ) < 6 )
  {
    fprintf( stderr, "Error parsing not after; malformed date." );
    return 7;
  }

  not_before_tm.tm_year += 100;
  not_after_tm.tm_year += 100;
  not_before_tm.tm_mon -= 1;
  not_after_tm.tm_mon -= 1;

  // TODO account for TZ information on end
  target->notBefore = mktime( &not_before_tm );
  target->notAfter = mktime( &not_after_tm );
 
  return 0;
} 

static const unsigned char OID_RSA[] = 
  { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x01 };
static const unsigned char OID_DSA[] = 
  { 0x2A, 0x86, 0x48, 0xCE, 0x38, 0x04, 0x01 };

static int parse_dsa_params( public_key_info *target, struct asn1struct *source )
{
 struct asn1struct *p;
 struct asn1struct *q;
 struct asn1struct *g;
 
 p = source->children;
 q = p->next;
 g = q->next;
 
 parse_huge( &target->dsa_parameters.p, p );
 parse_huge( &target->dsa_parameters.q, q );
 parse_huge( &target->dsa_parameters.g, g );

 return 0;
}

static int parse_public_key_info( public_key_info *target, 
                 struct asn1struct *source )
{ 
  struct asn1struct *oid;
  struct asn1struct *public_key;
  struct asn1struct public_key_value;

  if ( !validate_node( source, ASN1_SEQUENCE, 2, "public key info" ) )
  {
    return 1;
  }

  if ( !validate_node( source->children, ASN1_SEQUENCE, 1, "public key OID" ) )
  {
    return 2;
  }

  oid = source->children->children;
  public_key = source->children->next;

  if ( !validate_node( oid, ASN1_OBJECT_IDENTIFIER, 0, "public key OID" ) )
  {
    return 3;
  }

  if ( !validate_node( public_key, ASN1_BIT_STRING, 0, "public key info" ) )
  {
    return 4;
  }
 
  // The public key is a bit string encoding yet another ASN.1 DER-encoded
  // value - need to parse *that* here
  // Skip over the "0" byte in the public key.
  if ( asn1parse( public_key->data + 1, 
          public_key->length - 1, 
          &public_key_value ) )
  { 
    fprintf( stderr, 
      "Error; public key node is malformed (not ASN.1 DER-encoded)\n" );
    return 5;
  }
  
  if ( !memcmp( oid->data, &OID_RSA, sizeof( OID_RSA ) ) )
  {
    target->algorithm = rsa;

    if ( !validate_node( &public_key_value, ASN1_SEQUENCE, 2, "RSA public key value" ) )
    {
      return 6;
    }

    parse_huge( target->rsa_public_key.modulus, public_key_value.children );
    parse_huge( target->rsa_public_key.exponent, public_key_value.children->next );
    // This is important. Most times, the response includes a trailing 0 byte
    // to stop implementations from interpreting it as a twos-complement
    // negative number. However, in this implementation, this causes the
    // results to be the wrong size, so they need to be contracted.
    contract( target->rsa_public_key.modulus );
    contract( target->rsa_public_key.exponent );
  }
  else if ( !memcmp( oid->data, &OID_DSA, sizeof( OID_DSA ) ) )
  {
    struct asn1struct *params;
    target->algorithm = dsa;

    if ( !validate_node( &public_key_value, ASN1_INTEGER, 0, "DSA public key value" ) )
    {
      return 6;
    }
   
    parse_huge( &target->dsa_public_key, &public_key_value ); 

    params = oid->next;
    
    if ( !validate_node( params, ASN1_SEQUENCE, 3, "DSA public key params" ) )
    {
      return 6;
    }

    parse_dsa_params( target, params );
  }
  else
  {
    fprintf( stderr, "Error; unsupported OID in public key info.\n" );
    return 7;
  }

  asn1free( &public_key_value );

  return 0;
}

int asn1_get_bit( const int length,
         const unsigned char *bit_string,
         const int bit )
{
  if ( bit > ( ( length - 1 ) * 8 ) )
  {
    return 0;
  }
  else
  {
    return bit_string[ 1 + ( bit / 8 ) ] & ( 0x80 >> ( bit % 8 ) );
  }
}

static const unsigned char OID_keyUsage[] = { 0x55, 0x1D, 0x0F };
#define BIT_CERT_SIGNER 5

static int parse_extension( x509_certificate *certificate,
              struct asn1struct *source )
{
  struct asn1struct *oid;
  struct asn1struct *critical;
  struct asn1struct *data;

  oid = ( struct asn1struct * ) source->children;
  critical = ( struct asn1struct * ) oid->next;
  if ( critical->tag == ASN1_BOOLEAN )
  {
    data = ( struct asn1struct * ) critical->next;
  }
  else
  {
    // critical defaults to false
    data = critical;
    critical = NULL;
  }
  if ( !memcmp( oid->data, OID_keyUsage, oid->length ) )
  {
    struct asn1struct key_usage_bit_string;
    asn1parse( data->data, data->length, &key_usage_bit_string );
    if ( asn1_get_bit( key_usage_bit_string.length, 
              key_usage_bit_string.data, 
              BIT_CERT_SIGNER ) )
    {
      certificate->certificate_authority = 1;
    }
    asn1free( &key_usage_bit_string );
  } 
  // TODO recognize and parse extensions ?there are several

  return 0;
}

static int parse_extensions( x509_certificate *certificate,
               struct asn1struct *source )
{
  // Parse each extension; if one is recognized, update the certificate
  // in some way
  source = source->children->children;
  while ( source )
  {
    if ( parse_extension( certificate, source ) )
    {
      return 1;
    }
    source = source->next;
  }

  return 0;
}

static int parse_tbs_certificate( x509_certificate *target, 
                 struct asn1struct *source )
{
  struct asn1struct *version;
  struct asn1struct *serialNumber;
  struct asn1struct *signatureAlgorithmIdentifier;
  struct asn1struct *issuer;
  struct asn1struct *validity;
  struct asn1struct *subject;
  struct asn1struct *publicKeyInfo;
  struct asn1struct *extensions;

  if ( !validate_node( source, ASN1_SEQUENCE, 6, "TBS certificate" ) )
  {
    return 2;
  }

  // Figure out if there's an explicit version or not; if there is, then everything
  // else "shifts down" one spot.
  version = ( struct asn1struct * ) source->children;
 
  if ( version->tag == 0 && version->tag_class == ASN1_CONTEXT_SPECIFIC )
  {
     struct asn1struct *versionNumber = 
     ( struct asn1struct * ) version->children;  

    if ( !validate_node( versionNumber, ASN1_INTEGER, 0, "version number" ) )
    {
      return 2;
    }

    // This will only ever be one byte; safe
    target->version = ( *versionNumber->data ) + 1;    
    serialNumber = ( struct asn1struct * ) version->next;
  } 
  else
  {
    target->version = 1; // default if not provided
    serialNumber = ( struct asn1struct * ) version;
  } 
 
  signatureAlgorithmIdentifier = ( struct asn1struct * ) serialNumber->next;
  issuer = ( struct asn1struct * ) signatureAlgorithmIdentifier->next;
  validity = ( struct asn1struct * ) issuer->next;
  subject = ( struct asn1struct * ) validity->next;
  publicKeyInfo = ( struct asn1struct * ) subject->next;
  extensions = ( struct asn1struct * ) publicKeyInfo->next;
 
  if ( parse_huge( &target->serialNumber, serialNumber ) ) { return 2; } //解析序列号
  if ( parse_algorithm_identifier( &target->signature,               //解析算法标识
                   signatureAlgorithmIdentifier ) )
   { return 3; }
  if ( parse_name( &target->issuer, issuer ) ) { return 4; }             //解析发行者   
  if ( parse_validity( &target->validity, validity ) ) { return 5; }    //解析有效期
  if ( parse_name( &target->subject, subject ) ) { return 6; }          //解析主体信息
  if ( parse_public_key_info( &target->subjectPublicKeyInfo, publicKeyInfo ) )  //解析公钥信息
   { return 7; }
  if ( extensions )
  {
    if ( parse_extensions( target, extensions ) ) { return 8; }   //解析扩展信息
  }
  
  return 0;
}

/**
 * An RSA signature is an ASN.1 DER-encoded PKCS-7 structure including
 * the OID of the signature algorithm (again), and the signature value.
 */
int validate_certificate_rsa( signed_x509_certificate *certificate,
                   rsa_key *public_key )
{
  unsigned char *pkcs7_signature_decrypted;
  int pkcs7_signature_len;
  struct asn1struct pkcs7_signature;
  struct asn1struct *hash_value;
  int valid = 0;

  pkcs7_signature_len = rsa_decrypt( certificate->rsa_signature_value.rep,
    certificate->rsa_signature_value.size, &pkcs7_signature_decrypted,
    public_key );

  if ( pkcs7_signature_len == -1 )
  {
    fprintf( stderr, "Unable to decode signature value.\n" );
    return valid;
  }
  if ( asn1parse( pkcs7_signature_decrypted, pkcs7_signature_len,
      &pkcs7_signature ) )
  {
    fprintf( stderr, "Unable to parse signature\n" );
    return valid;
  } 
  
  hash_value = pkcs7_signature.children->next;
 
  if ( memcmp( hash_value->data, certificate->hash, certificate->hash_len ) )
  {
    valid = 0;
  } 
  else
  {
    valid = 1;
  }
 
  asn1free( &pkcs7_signature );
 
  return valid;
}

int validate_certificate_dsa( signed_x509_certificate *certificate )
{
 return dsa_verify(
  &certificate->tbsCertificate.subjectPublicKeyInfo.dsa_parameters,
  &certificate->tbsCertificate.subjectPublicKeyInfo.dsa_public_key,
  certificate->hash,
  certificate->hash_len * 4,
  &certificate->dsa_signature_value );
}

/**
 * If the signature type is RSA, the signature value is a bit string
 * which should be interpreted as a single number.
 */
static int parse_rsa_signature_value( signed_x509_certificate *target, 
                                      struct asn1struct *source )
{
  parse_huge( &target->rsa_signature_value, source );
  contract( &target->rsa_signature_value );

  return 0;
}

static int parse_dsa_signature_value( signed_x509_certificate *target,
                   struct asn1struct *source )
{
 struct asn1struct dsa_signature;

 if ( asn1parse( source->data + 1, source->length - 1, &dsa_signature ) )
 {
  fprintf( stderr, "Unable to parse ASN.1 DER-encoded signature.\n" );
  return 1;
 }

 parse_huge( &target->dsa_signature_value.r, dsa_signature.children );
 parse_huge( &target->dsa_signature_value.s, dsa_signature.children->next );

 asn1free( &dsa_signature );

 return 0;
}

int parse_x509_certificate( const unsigned char *buffer,
              const unsigned int certificate_length,
              signed_x509_certificate *parsed_certificate )   // 正确的返回0, 错误的返回42。
{
  struct asn1struct certificate;
  struct asn1struct *tbsCertificate;
  struct asn1struct *algorithmIdentifier;
  struct asn1struct *signatureValue;
  digest_ctx digest;

  // First, read the whole thing into a traversable ASN.1 structure
  asn1parse( buffer, certificate_length, &certificate );  

  // Version can be implicit or explicit
  tbsCertificate = ( struct asn1struct * ) certificate.children;       //解析第一块证书主体内容

  algorithmIdentifier = ( struct asn1struct * ) tbsCertificate->next;  //算法标志
  signatureValue = ( struct asn1struct * ) algorithmIdentifier->next;  //第三块签名的值
  if ( parse_tbs_certificate( &parsed_certificate->tbsCertificate,     //解析证书主体
     tbsCertificate ) )
  { 
    fprintf( stderr, "Error trying to parse TBS certificate\n" );
    return 42;
  }
  if ( parse_algorithm_identifier( &parsed_certificate->algorithm,    //解析算法标志
                  algorithmIdentifier ) )
  {
    return 42;
  } 

  switch ( parsed_certificate->algorithm ) 
  {
   case md5WithRSAEncryption:
   case shaWithRSAEncryption:
     if ( parse_rsa_signature_value( parsed_certificate, signatureValue ) )    //解析签名值
     {
       return 42;
     }
    break;
   case shaWithDSA:
     if ( parse_dsa_signature_value( parsed_certificate, signatureValue ) )
     {
       return 42;
     }
   }

  switch ( parsed_certificate->algorithm )
  {
    case md5WithRSAEncryption:
      new_md5_digest( &digest ); // 对digest进行初始化
      break;
    case shaWithRSAEncryption:
    case shaWithDSA:
      new_sha1_digest( &digest );
      break;
    default:
      break;
  }

  update_digest( &digest, tbsCertificate->data, tbsCertificate->length );
  finalize_digest( &digest );

  parsed_certificate->hash = digest.hash;
  parsed_certificate->hash_len = digest.hash_len;

  asn1free( &certificate );

  return 0;
}

static void output_x500_name( name *x500_name )
{
  printf( "C=%s/ST=%s/L=%s/O=%s/OU=%s/CN=%s\n",
    ( x500_name->idAtCountryName ? x500_name->idAtCountryName : "?" ),
    ( x500_name->idAtStateOrProvinceName ? x500_name->idAtStateOrProvinceName : "?" ),
    ( x500_name->idAtLocalityName ? x500_name->idAtLocalityName : "?" ),
    ( x500_name->idAtOrganizationName ? x500_name->idAtOrganizationName : "?" ),
    ( x500_name->idAtOrganizationalUnitName ? x500_name->idAtOrganizationalUnitName : "?" ),
    ( x500_name->idAtCommonName ? x500_name->idAtCommonName : "?" ) );
}

static void print_huge( huge *h, CString& str)
{
  show_hex_str( h->rep, h->size, str);
}


static void print_huge2( huge *h, char *str)
{
	show_hex_char( h->rep, h->size, str);
}




 void display_x509_certificate( signed_x509_certificate *certificate, CString& str)
{

  char tmp[255];
  printf( "Certificate details:\n" );

  printf( "Version: %d\n", certificate->tbsCertificate.version );
  sprintf(tmp, "Version: %d\r\n", certificate->tbsCertificate.version);
  str += (CString)tmp;

  printf( "Serial number: " );
  str += "Serial number: ";
  print_huge( &certificate->tbsCertificate.serialNumber,str);
  str +="\r\n";

  printf( "issuer: " );
  str += "Issuer :\r\n";
  output_x500_name( &certificate->tbsCertificate.issuer );
  name *x500_name;
  x500_name = &certificate->tbsCertificate.issuer;
  sprintf(tmp,"");
  sprintf(tmp, " C=%s\r\n ST=%s\r\n L=%s\r\n O=%s\r\n OU=%s\r\n CN=%s\r\n",
	  ( x500_name->idAtCountryName ? x500_name->idAtCountryName : "?" ),
	  ( x500_name->idAtStateOrProvinceName ? x500_name->idAtStateOrProvinceName : "?" ),
	  ( x500_name->idAtLocalityName ? x500_name->idAtLocalityName : "?" ),
	  ( x500_name->idAtOrganizationName ? x500_name->idAtOrganizationName : "?" ),
	  ( x500_name->idAtOrganizationalUnitName ? x500_name->idAtOrganizationalUnitName : "?" ),
	  ( x500_name->idAtCommonName ? x500_name->idAtCommonName : "?" ) );
  str += (CString)tmp;
  str +="\r\n";
  printf( "subject : " );
  str += "Subject :\r\n";
  output_x500_name( &certificate->tbsCertificate.subject );
  x500_name = &certificate->tbsCertificate.subject;
  sprintf(tmp,"");
  sprintf(tmp, " C=%s\r\n ST=%s\r\n L=%s\r\n O=%s\r\n OU=%s\r\n CN=%s\r\n",
	  ( x500_name->idAtCountryName ? x500_name->idAtCountryName : "?" ),
	  ( x500_name->idAtStateOrProvinceName ? x500_name->idAtStateOrProvinceName : "?" ),
	  ( x500_name->idAtLocalityName ? x500_name->idAtLocalityName : "?" ),
	  ( x500_name->idAtOrganizationName ? x500_name->idAtOrganizationName : "?" ),
	  ( x500_name->idAtOrganizationalUnitName ? x500_name->idAtOrganizationalUnitName : "?" ),
	  ( x500_name->idAtCommonName ? x500_name->idAtCommonName : "?" ) );
  str += (CString)tmp;
 
  str +="\r\n";
  sprintf(tmp,"");
  printf( "not before: %s", asctime( gmtime(
   &certificate->tbsCertificate.validity.notBefore ) ) );
  sprintf(tmp,"Not before: %s\r\n", asctime( gmtime(
	  &certificate->tbsCertificate.validity.notBefore ) ) );
  str += (CString)tmp;


  sprintf(tmp,"");
  printf( "not after: %s", asctime( gmtime(
   &certificate->tbsCertificate.validity.notAfter ) ) );
  sprintf(tmp, "Not after: %s\r\n", asctime( gmtime(
	  &certificate->tbsCertificate.validity.notAfter ) ) );
  str += (CString)tmp;


  str +="\r\n";
  printf( "Public key algorithm: " );
  str += "Public key algorithm: "; 
  switch ( certificate->tbsCertificate.subjectPublicKeyInfo.algorithm )
  {
    case rsa:
      printf( "RSA\n" );
      printf( "modulus: " );
	  str += " RSA\r\n";
	  str += "  modulus: ";

      print_huge( 
        certificate->tbsCertificate.subjectPublicKeyInfo.rsa_public_key.modulus ,str);

	  str += "  exponent: ";
      printf( "exponent: " );
      print_huge( 
        certificate->tbsCertificate.subjectPublicKeyInfo.rsa_public_key.exponent,str);
      break;
  case dsa:
   printf( "DSA\n" );
   str += "DSA\r\n";
   printf( "y: " );
   str += "  y:";
   print_huge( 
    &certificate->tbsCertificate.subjectPublicKeyInfo.dsa_public_key,str );
   printf( "p: " );
   str += "  p:";
   print_huge( 
    &certificate->tbsCertificate.subjectPublicKeyInfo.dsa_parameters.p ,str);
   printf( "q: " );
   str += "  q:";
   print_huge( 
    &certificate->tbsCertificate.subjectPublicKeyInfo.dsa_parameters.q ,str);
   printf( "g: " );
   str += "  g";
   print_huge( 
    &certificate->tbsCertificate.subjectPublicKeyInfo.dsa_parameters.g ,str);
   break;
    case dh:
      printf( "DH\n" );
	  str += "DH\r\n";
      break;
    default:
      printf( "?\n" );
	  str += "?\r\n";
      break;
  }

  printf( "Signature algorithm: " );
  str += "Signature algorithm:  " ;

  switch ( certificate->algorithm )
  {
    case md5WithRSAEncryption:
      printf( "MD5 with RSA Encryption\n" );
	  str += "MD5 with RSA Encryption\r\n";
      break;
  case shaWithDSA:
   printf( "SHA-1 with DSA\n" );
    str += "SHA-1 with DSA\r\n";
   break;
    case shaWithRSAEncryption:
      printf( "SHA-1 with RSA Encryption\n" );
	   str += "SHA-1 with RSA Encryption\r\n";
      break;
  }
 
  printf( "Signature value: " );
  str += "Signature value:  ";
 
  switch ( certificate->algorithm )
  {
    case md5WithRSAEncryption:
    case shaWithRSAEncryption:
      print_huge( &certificate->rsa_signature_value,str );
      break;
  case shaWithDSA:
   printf( "\n\tr:" );
   str += " r:";
   print_huge( &certificate->dsa_signature_value.r ,str);
   printf( "\ts:" );
   str += " s:";
   print_huge( &certificate->dsa_signature_value.s, str);
   break;
  }
  printf( "\n" );
 
  str +="\r\n";
  if ( certificate->tbsCertificate.certificate_authority )
  {
    printf( "is a CA\n" );
	str += "This is a CA\r\n";
  } 
  else
  {
    printf( "is not a CA\n" );
	str += "This is not a CA\r\n";
  } 

} 



void display_x509( signed_x509_certificate *certificate, x509Info *x509_msg)
{
    int len = 0;
	printf( "Version: %d\n", certificate->tbsCertificate.version );
	len = strlen( "Version: %d\n");
    x509_msg->version = (char*)malloc(len+1);
	x509_msg->version[len] = 0;  //写入的是6个字符+1个结束符
	sprintf(x509_msg->version, "Version: %d\n", certificate->tbsCertificate.version);

	printf( "Serial number: " );
	len = certificate->tbsCertificate.serialNumber.size * 2;
	x509_msg->serialnumber = (char*)malloc(len+1);
    x509_msg->serialnumber[len] = 0;
	print_huge2( &certificate->tbsCertificate.serialNumber,x509_msg->serialnumber);

	printf( "issuer: " );
	output_x500_name( &certificate->tbsCertificate.issuer );
	name *x500_name;
	x500_name = &certificate->tbsCertificate.issuer;
 
    len = 0;
    if(x500_name->idAtCountryName)
	{
		len += strlen(x500_name->idAtCountryName);
	}
    if(x500_name->idAtStateOrProvinceName)
	{
		len += strlen(x500_name->idAtStateOrProvinceName);
	}

    if(x500_name->idAtLocalityName)
	{
		len += strlen(x500_name->idAtLocalityName);
	}

   if(x500_name->idAtOrganizationName)
   {
       len += strlen(x500_name->idAtOrganizationName);
   }
   
   if(x500_name->idAtOrganizationalUnitName)
   {
	   len += strlen(x500_name->idAtOrganizationalUnitName);
   }

   if(x500_name->idAtCommonName)
   {
     len += strlen(x500_name->idAtCommonName);
   }

     len += strlen( " C=%s\r\n ST=%s\r\n L=%s\r\n O=%s\r\n OU=%s\r\n CN=%s\r\n");


	x509_msg->issuer = (char*) malloc(len+1);
	x509_msg->issuer[len] = 0;

	sprintf(x509_msg->issuer, " C=%s\r\n ST=%s\r\n L=%s\r\n O=%s\r\n OU=%s\r\n CN=%s\r\n",
		( x500_name->idAtCountryName ? x500_name->idAtCountryName : "?" ),
		( x500_name->idAtStateOrProvinceName ? x500_name->idAtStateOrProvinceName : "?" ),
		( x500_name->idAtLocalityName ? x500_name->idAtLocalityName : "?" ),
		( x500_name->idAtOrganizationName ? x500_name->idAtOrganizationName : "?" ),
		( x500_name->idAtOrganizationalUnitName ? x500_name->idAtOrganizationalUnitName : "?" ),
		( x500_name->idAtCommonName ? x500_name->idAtCommonName : "?" ) );


	printf( "subject : " );
	output_x500_name( &certificate->tbsCertificate.subject );
	x500_name = &certificate->tbsCertificate.subject;

	len = 0;
	if(x500_name->idAtCountryName)
	{
		len += strlen(x500_name->idAtCountryName);
	}
	if(x500_name->idAtStateOrProvinceName)
	{
		len += strlen(x500_name->idAtStateOrProvinceName);
	}

	if(x500_name->idAtLocalityName)
	{
		len += strlen(x500_name->idAtLocalityName);
	}

	if(x500_name->idAtOrganizationName)
	{
		len += strlen(x500_name->idAtOrganizationName);
	}

	if(x500_name->idAtOrganizationalUnitName)
	{
		len += strlen(x500_name->idAtOrganizationalUnitName);
	}

	if(x500_name->idAtCommonName)
	{
		len += strlen(x500_name->idAtCommonName)+1;
	}
	len += strlen( " C=%s\r\n ST=%s\r\n L=%s\r\n O=%s\r\n OU=%s\r\n CN=%s\r\n");

	x509_msg->subject = (char*) malloc(len + 1);
	x509_msg->subject[len] = 0;

	sprintf(x509_msg->subject, " C=%s\r\n ST=%s\r\n L=%s\r\n O=%s\r\n OU=%s\r\n CN=%s\r\n",
		( x500_name->idAtCountryName ? x500_name->idAtCountryName : "?" ),
		( x500_name->idAtStateOrProvinceName ? x500_name->idAtStateOrProvinceName : "?" ),
		( x500_name->idAtLocalityName ? x500_name->idAtLocalityName : "?" ),
		( x500_name->idAtOrganizationName ? x500_name->idAtOrganizationName : "?" ),
		( x500_name->idAtOrganizationalUnitName ? x500_name->idAtOrganizationalUnitName : "?" ),
		( x500_name->idAtCommonName ? x500_name->idAtCommonName : "?" ) );



	printf( "%s", asctime( gmtime(
		&certificate->tbsCertificate.validity.notBefore ) ) );
	len = 30;
	x509_msg->notbefore = (char*) malloc(len + 1);
	x509_msg->notbefore[len] = 0;
	sprintf(x509_msg->notbefore,"%s\r\n", asctime( gmtime(
		&certificate->tbsCertificate.validity.notBefore ) ) );

	printf( "%s", asctime( gmtime(
		&certificate->tbsCertificate.validity.notAfter ) ) );
    len = 30;
	x509_msg->notafter = (char*) malloc(len + 1);
	x509_msg->notafter[len] = 0;
	sprintf(x509_msg->notafter, "%s\r\n", asctime( gmtime(
		&certificate->tbsCertificate.validity.notAfter ) ) );

	printf( "Public key algorithm: " );
	switch ( certificate->tbsCertificate.subjectPublicKeyInfo.algorithm )
	{
	case rsa:
		printf( "RSA\n" );
		len = 4;
		x509_msg->algFlag = (char *)malloc(len+1);
		x509_msg->algFlag[len] = 0;
		sprintf(x509_msg->algFlag, "RSA");
		printf( "modulus: " );

		len = certificate->tbsCertificate.subjectPublicKeyInfo.rsa_public_key.modulus->size * 2;
        x509_msg->rs.modulus =(char *) malloc(len+1);
		x509_msg->rs.modulus[len] = 0;
		print_huge2( 
			certificate->tbsCertificate.subjectPublicKeyInfo.rsa_public_key.modulus ,x509_msg->rs.modulus);

		printf( "exponent: " );
		 len  = certificate->tbsCertificate.subjectPublicKeyInfo.rsa_public_key.exponent->size * 2;
		x509_msg->rs.exponent = (char *) malloc(len+ 1);
	   	x509_msg->rs.exponent[len] = 0;
		
		print_huge2( 
			certificate->tbsCertificate.subjectPublicKeyInfo.rsa_public_key.exponent,x509_msg->rs.exponent);
		break;
	case dsa:
		printf( "DSA\n" );
		len = 4;
		x509_msg->algFlag = (char *)malloc(len+1);
		x509_msg->algFlag[len] = 0;
		sprintf(x509_msg->algFlag, "DSA");
		printf( "y: " );
		len = certificate->tbsCertificate.subjectPublicKeyInfo.dsa_public_key.size * 2;
		x509_msg->ds.y = (char *)malloc(len + 1);
		x509_msg->ds.y[len] = 0;
		print_huge2( 
			&certificate->tbsCertificate.subjectPublicKeyInfo.dsa_public_key,x509_msg->ds.y );
		printf( "p: " );
		len = certificate->tbsCertificate.subjectPublicKeyInfo.dsa_parameters.p.size * 2;
		x509_msg->ds.p = (char *)malloc( len + 1);
		x509_msg->ds.p[len] = 0;
		print_huge2( 
			&certificate->tbsCertificate.subjectPublicKeyInfo.dsa_parameters.p,x509_msg->ds.p);
		printf( "q: " );
		len = certificate->tbsCertificate.subjectPublicKeyInfo.dsa_parameters.q.size * 2;
		x509_msg->ds.q = (char *)malloc( len + 1);
		x509_msg->ds.q[len] = 0;
		print_huge2( 
			&certificate->tbsCertificate.subjectPublicKeyInfo.dsa_parameters.q ,x509_msg->ds.q);
		printf( "g: " );
		len = certificate->tbsCertificate.subjectPublicKeyInfo.dsa_parameters.g.size*2;
		x509_msg->ds.g =(char *) malloc(len + 1);
		x509_msg->ds.g[len] = 0;
		print_huge2( 
			&certificate->tbsCertificate.subjectPublicKeyInfo.dsa_parameters.g ,x509_msg->ds.g);
		break;
	case dh:
		printf( "DH\n" );
		break;
	default:
		printf( "?\n" );
		break;
	}

	printf( "Signature algorithm: " );
	len = 60;
	x509_msg->signAlgorithm = (char *)malloc(len+ 1);
	x509_msg->signAlgorithm[len] = 0;
	switch ( certificate->algorithm )
	{
	case md5WithRSAEncryption:
		printf( "MD5 with RSA Encryption\n" );
		sprintf(x509_msg->signAlgorithm, "MD5 with RSA Encryption\n");
		break;
	case shaWithDSA:
		printf( "SHA-1 with DSA\n" );
	    sprintf(x509_msg->signAlgorithm, "SHA-1 with DSA\n");
		break;
	case shaWithRSAEncryption:
		printf( "SHA-1 with RSA Encryption\n" );
	    sprintf(x509_msg->signAlgorithm, "SHA-1 with RSA Encryption\n");
		break;
	}

	printf( "Signature value: " );

	switch ( certificate->algorithm )
	{
	case md5WithRSAEncryption:
	case shaWithRSAEncryption:
		len = certificate->rsa_signature_value.size * 2 ;
         x509_msg->rs.signValue = (char *) malloc( len + 1);
		 x509_msg->rs.signValue[len] = 0;
		print_huge2( &certificate->rsa_signature_value,x509_msg->rs.signValue);
		break;
	case shaWithDSA:
		printf( "\n\tr:" );
		len = certificate->dsa_signature_value.r.size * 2;
        x509_msg->ds.r = (char *)malloc( len + 1);
		x509_msg->ds.r[len] = 0;
		print_huge2( &certificate->dsa_signature_value.r ,x509_msg->ds.r);
		printf( "\ts:" );
		len = certificate->dsa_signature_value.s.size * 2;
		x509_msg->ds.s = (char *)malloc(len + 1);
		x509_msg->ds.s[len] = 0;
		print_huge2( &certificate->dsa_signature_value.s, x509_msg->ds.s);
		break;
	}
	printf( "\n" );




     len = 30;
	x509_msg->caflag = (char *)malloc(len + 1);
	x509_msg->caflag[len] = 0;
	if ( certificate->tbsCertificate.certificate_authority )
	{
		printf( "is a CA\n" );
		sprintf(x509_msg->caflag, "This is a CA\n");
	} 
	else
	{
		printf( "is not a CA\n" );
	  sprintf(x509_msg->caflag, "This is not a CA\n");
	} 

} 


#ifdef TEST_X509
int main( int argc, char *argv[ ] )
{
  int certificate_file;
  struct stat certificate_file_stat;  // struct stat这个结构体是用来描述一个linux系统文件系统中的文件属性的结构。
  char *buffer, *bufptr;
  int buffer_size;
  int bytes_read;
  int error_code;
 
  signed_x509_certificate certificate;
 
  if ( argc < 3 )
  {
    fprintf( stderr, "Usage: x509 [-pem|-der] [certificate file]\n" );
    exit( 0 );
  }

  if ( ( certificate_file = open( argv[ 2 ], O_RDONLY ) ) == -1 )
  {
    perror( "Unable to open certificate file" );
    return 1;
  }

  // Slurp the whole thing into memory
  if ( fstat( certificate_file, &certificate_file_stat ) )
  {
    perror( "Unable to stat certificate file" );
    return 2;
  } 
 
  buffer_size = certificate_file_stat.st_size;
  buffer = ( char * ) malloc( buffer_size );
  if ( !buffer )
  {
    perror( "Not enough memory" );
    return 3;
  } 
 
  bufptr = buffer;
 
  while ( ( bytes_read = read( certificate_file, ( void * ) buffer, 
                 buffer_size ) ) )
  { 
    bufptr += bytes_read;
  }
  
  if ( !strcmp( argv[ 1 ], "-pem" ) )
  { 
    // XXX this overallocates a bit, since it sets aside space for markers, etc.
    unsigned char *pem_buffer = buffer;
    buffer = (unsigned char * ) malloc( buffer_size );
    buffer_size = pem_decode( pem_buffer, buffer );
    free( pem_buffer );
  }

  // now parse it
  init_x509_certificate( &certificate );
  if ( !( error_code = parse_x509_certificate( buffer, buffer_size, 
                         &certificate ) ) )
  {
    printf( "X509 Certificate:\n" );
    display_x509_certificate( &certificate );

    // Assume it's a self-signed certificate and try to validate it that
    switch ( certificate.algorithm )
    {
     case md5WithRSAEncryption:
     case shaWithRSAEncryption:
       if ( validate_certificate_rsa( &certificate,
        &certificate.tbsCertificate.subjectPublicKeyInfo.rsa_public_key ) )
       {
         printf( "Certificate is a valid self-signed certificate.\n" );
       }
       else
       {
         printf( "Certificate is corrupt or not self-signed.\n" );
       }
       break;
   case shaWithDSA:
    if ( validate_certificate_dsa( &certificate ) )
    {
     printf( "Certificate is a valid self-signed certificate.\n" );
    }
    else
    {
     printf( "Certificate is corrupt or not self-signed.\n" );
    }
    }
  }
  else
  {
    printf( "error parsing certificate: %d\n", error_code );
  }

  free_x509_certificate( &certificate );
  free( buffer );
  return 0;
}
#endif
